// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <expat.h>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <iostream>
#include "element.h"
#include "err.h"
#include "utils.h"

const char* arg0;

using namespace std;

const string prefix = ".";

#define TRACE 1

bool GenerateFunction(const Element& elem, const string& path, ostream& os)
{
    const string& refid = elem.attrs()["refid"];
    const string& name = elem["name"].chars();
    const string link = refid + ".md";

    os << "* ";
    os << "[" << name << "()]";
    os << "(" << link << ")";
    os << endl << endl;

    return true;
}

enum Trait
{
    BOLD,
    EMPHASIS,
    VERBATIM,
    REF,
};

void SubstituteTrait(
    string& chars,
    Trait trait,
    const string& text,
    const string& arg = string())
{
    if (trait == VERBATIM)
    {
        chars += "\n\n```\n" + text + "\n```\n\n";
        return;
    }

    size_t pos1 = chars.find(" ,");
    size_t pos3 = chars.find(" .");
    size_t pos2 = chars.find("  ");

    string s;

    if (trait == BOLD)
        s = "**" + text + "**";
    else if (trait == EMPHASIS)
        s = "*" + text + "*";
    else if (trait == REF)
        s = "[" + text + "](" + arg + ".md)";

    size_t pos = min(min(pos1, pos2), pos3);

    if (pos == string::npos)
        chars += ' ' + s;
    else
        chars = chars.substr(0, pos) + ' ' + s + chars.substr(pos + 1);
}

bool PrintPara(const Element& elem, ostream& os)
{
    string chars = elem.chars();

    // Perform substitutions on trait elements:
    for (size_t i = 0; i < elem.size(); i++)
    {
        const Element& e = elem[i];

        if (e.name() == "bold")
        {
            SubstituteTrait(chars, BOLD, e.chars());
        }
        else if (e.name() == "emphasis")
        {
            SubstituteTrait(chars, EMPHASIS, e.chars());
        }
        else if (e.name() == "verbatim")
        {
            SubstituteTrait(chars, VERBATIM, e.chars());
        }
        if (e.name() == "ref")
        {
            string refid = e.attrs()["refid"];
            SubstituteTrait(chars, REF, e.chars(), refid);
        }
        else if (e.name() == "itemizedlist")
        {
            os << chars << endl << endl;

            vector<Element> listitems;
            e.find("listitem", listitems);

            for (size_t i = 0; i < listitems.size(); i++)
            {
                const Element& tmp_para = listitems[i]["para"];

                os << "- ";

                if (!PrintPara(tmp_para, os))
                    return false;
            }

            return true;
        }
    }

    os << chars << endl << endl;

    return true;
}

bool PrintParameterList(const Element& elem, ostream& os)
{
    const string& kind = elem.attrs()["kind"];

    if (kind == "param")
        os << "## Parameters" << endl << endl;
    else if (kind == "retval")
        os << "## Return value" << endl << endl;
    else
        return false;

    vector<Element> parameteritems;
    elem.find("parameteritem", parameteritems);

    for (size_t i = 0; i < parameteritems.size(); i++)
    {
        const Element& pi = parameteritems[i];
        string name = pi["parameternamelist"]["parametername"].chars();

        if (name.size() == 0)
            return false;

        os << "#### " << name << endl << endl;

        Element para = pi["parameterdescription"]["para"];
        if (!PrintPara(para, os))
            return false;
    }

    return true;
}

bool PrintSimpleSect(const Element& elem, ostream& os)
{
    const string& kind = elem.attrs()["kind"];

    if (kind == "return")
        os << "## Returns" << endl << endl;
    else
        return false;

    Element para = elem["para"];

    if (!PrintPara(para, os))
        return false;

    return true;
}

bool PrintDetailedDescription(const Element& detaileddescription, ostream& os)
{
    vector<Element> paras;
    detaileddescription.find("para", paras);

    for (size_t i = 0; i < paras.size(); i++)
    {
        const Element& para = paras[i];

        // Print the detailed description paragraphs:
        if (!PrintPara(para, os))
            return false;

        // Handle 'parameterlist' and 'simplesect' children:
        for (size_t j = 0; j < para.size(); j++)
        {
            const Element& child = para[j];

            if (child.name() == "parameterlist")
            {
                if (!PrintParameterList(child, os))
                    return false;
            }

            if (child.name() == "simplesect")
            {
                if (!PrintSimpleSect(child, os))
                    return false;
            }
        }
    }

    return true;
}

bool GenerateFunctionFile(const Element& elem)
{
    const string& name = elem["name"].chars();
    const string& id = elem.attrs()["id"];

    // Open the file:

    string filename = prefix + "/" + id + ".md";

    ofstream os(filename.c_str());
    if (!os)
        return false;

    cout << "Created " << filename << endl;

    // Write the navigation back to index:
    os << "[Index](index.md)" << endl << endl;
    os << "---" << endl;

    // Write the function name:
    os << "# " << name << "()" << endl << endl;

    // Write the brief description:
    {
#if 0
        string desc = elem["briefdescription"]["para"].chars();
        os << desc << endl << endl;
#else
        Element para = elem["briefdescription"]["para"];

        // Print the detailed description paragraphs:
        if (!PrintPara(para, os))
            return false;
#endif
    }

    // Write syntax section:
    {
        os << "## Syntax" << endl << endl;

        // Print the return type:
        string type = elem["type"].chars();
        string args = elem["argsstring"].chars();

        // Possibly a ref?
        if (type.empty())
            type = elem["type"]["ref"].chars();

        os << "    " << type << " ";

#if 1
        os << name << args << endl;
#else
        os << name << "(" << endl;

        // Find all "param" elements:
        vector<Element> params;
        elem.find("param", params);

        for (size_t i = 0; i < params.size(); i++)
        {
            string type = params[i]["type"].chars();
            string declname = params[i]["declname"].chars();

            os << "        " << type;

            if (type.size() && type[type.size() - 1] != '*' && declname.size())
                os << ' ';

            os << declname;

            if (i + 1 == params.size())
                os << ");" << endl;
            else
                os << "," << endl;
        }
#endif
    }

    // Write the descriptions:
    {
        os << "## Description " << endl << endl;

        Element e = elem["detaileddescription"];

        if (!PrintDetailedDescription(e, os))
            return false;
    }

    // Write the navigation back to index:
    os << "---" << endl;
    os << "[Index](index.md)" << endl << endl;

    return true;
}

bool GenerateFile(const Element& elem, const string& path, ostream& os)
{
    Element root;

    // Parse the corresponding XML file:
    {
        const string& refid = elem.attrs()["refid"];
        string xmlfile = dirname(path) + '/' + refid + ".xml";

        ifstream is(xmlfile.c_str());

        if (!is)
        {
            err("failed to open %s", xmlfile.c_str());
            return false;
        }

        Error error;

        if (!Element::parse(xmlfile, root, error))
        {
            err("%u(%s): %s",
                error.line(),
                xmlfile.c_str(),
                error.message().c_str());
            return false;
        }

        // Dump the XML file easier to read form for debugging:
        {
            string dumpfile = prefix + '/' + refid + ".xml.dump";
            ofstream tmpos(dumpfile.c_str());

            if (tmpos)
            {
                root.dump(tmpos);
                cout << "Created " << dumpfile << endl;
            }
        }
    }

    // Generate file info:
    {
        const string& refid = elem.attrs()["refid"];
        const string& name = elem["name"].chars();
        const string link = refid + ".md";

#if 0
        os << "### [" << name << "]";
        os << "(" << link << ")";
        os << endl << endl;
#else
        os << "### " << name << endl << endl;
#endif
    }

    // Genereate functions:
    for (size_t i = 0; i < elem.size(); i++)
    {
        const Element& e = elem[i];
        const string& name = e["name"].chars();
        const string& kind = e.attrs()["kind"];
        const string& refid = e.attrs()["refid"];

        if (kind == "function")
        {
            Element f;

            if (!root.search("compounddef.sectiondef.memberdef.id", refid, f))
            {
                err("failed to find function: %s", name.c_str());
                return false;
            }

            string desc = f["briefdescription"]["para"].chars();

            if (desc.size() == 0)
                continue;

            if (!GenerateFunction(e, path, os))
                return false;

            if (!GenerateFunctionFile(f))
            {
                err("failed to generate function file for %s", name.c_str());
                return false;
            }
        }
    }

    return true;
}

bool GenerateIndex(const Element& root, const string& path)
{
    if (root.name() != "doxygenindex")
        return false;

    string filename = prefix + "/index.md";

    ofstream os(filename.c_str());

    if (!os)
        return false;

    cout << "Created " << filename << endl;

    os << "# Index" << endl << endl;

    // Generate structures:
    {
        os << "## Structures " << endl << endl;

        for (size_t i = 0; i < root.size(); i++)
        {
            const Element& e = root[i];
            const string& kind = e.attrs()["kind"];

            if (kind == "struct")
            {
                // ATTN:
            }
        }
    }

    // Generate files:
    {
        os << "## Files" << endl << endl;

        for (size_t i = 0; i < root.size(); i++)
        {
            const Element& e = root[i];
            const string& kind = e.attrs()["kind"];

            if (kind == "file")
            {
                if (!GenerateFile(e, path, os))
                    return false;
            }
        }
    }

    return true;
}

int main(int argc, const char* argv[])
{
    arg0 = argv[0];

    /* Check usage */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s XML-FILE\n", arg0);
        exit(1);
    }

    /* Parse the document into C++ tree */
    {
        Element root;
        Error error;

        if (!Element::parse(argv[1], root, error))
        {
            err("%u(%s): %s", error.line(), argv[1], error.message().c_str());
            exit(1);
        }

        /* Dump the index file to easier to read tree format for debugging */
        {
            string dumpfile = prefix + '/' + basename(argv[1]) + ".dump";
            ofstream tmpos(dumpfile.c_str());

            if (tmpos)
            {
                root.dump(tmpos);
                cout << "Created " << dumpfile << endl;
            }
        }

        if (!GenerateIndex(root, argv[1]))
        {
            err("GenerateIndex() failed");
        }
    }

    return 0;
}

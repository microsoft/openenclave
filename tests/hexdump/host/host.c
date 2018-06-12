// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <openenclave/host.h>
#include <openenclave/internal/hexdump.h>
#include <openenclave/internal/tests.h>
#include "../args.h"

/* Randomly generated data */
static const unsigned char _data[DATA_SIZE] = {
    0x47, 0x40, 0x8f, 0xe5, 0xef, 0x30, 0x2c, 0x72, 0x54, 0x4a, 0x43, 0x9b,
    0xec, 0xb8, 0x1c, 0x78, 0xd8, 0xe1, 0x54, 0xcd, 0x46, 0x3d, 0x2c, 0x83,
    0x62, 0x95, 0x0c, 0x85, 0x16, 0x3b, 0x22, 0xe6, 0x78, 0xa4, 0x5a, 0xac,
    0x3a, 0xe9, 0x03, 0x72, 0x85, 0xac, 0xe6, 0x0f, 0x93, 0x60, 0x78, 0xe0,
    0x82, 0x6b, 0xc1, 0x96, 0x51, 0x74, 0x34, 0xe8, 0x1b, 0xe7, 0xd2, 0x37,
    0xbb, 0x3f, 0x05, 0x23, 0x9a, 0x47, 0xa5, 0x32, 0x0f, 0x0a, 0x2e, 0xaa,
    0x9f, 0xb6, 0xea, 0x4c, 0x3b, 0xfa, 0xf9, 0x87, 0xf0, 0xbe, 0xd3, 0xdc,
    0xf5, 0x04, 0xc1, 0x4a, 0x4b, 0x03, 0x8e, 0x4e, 0x2b, 0xfb, 0x64, 0x66,
    0x3e, 0xed, 0x2b, 0x3c, 0x86, 0x09, 0x46, 0xc6, 0xb2, 0x40, 0x71, 0xdd,
    0xa1, 0x5a, 0x7d, 0xe0, 0x52, 0xa5, 0x91, 0xa5, 0xab, 0xe7, 0x87, 0x00,
    0x51, 0x3e, 0x5a, 0x79, 0x94, 0x15, 0xe7, 0x27, 0x7f, 0x73, 0x0a, 0x01,
    0x7f, 0x8b, 0x32, 0xa8, 0xb2, 0x32, 0x88, 0xd9, 0x0f, 0x67, 0x51, 0x84,
    0xd3, 0xd6, 0x91, 0x6c, 0x18, 0xe7, 0xa8, 0x1a, 0x96, 0x65, 0xe8, 0xbf,
    0x06, 0xa1, 0xef, 0xb7, 0x9f, 0xca, 0xe2, 0xf1, 0x50, 0xc6, 0xe2, 0x46,
    0x16, 0xc0, 0x69, 0x5b, 0xa6, 0x36, 0x1c, 0x0f, 0x0e, 0x4f, 0xc8, 0x13,
    0x49, 0x12, 0x47, 0x28, 0xe1, 0xcc, 0x3b, 0x99, 0x66, 0x37, 0xfc, 0xf8,
    0x3f, 0x4f, 0x48, 0xe9, 0xc4, 0x8e, 0x55, 0x32, 0xfa, 0xc3, 0x24, 0xe6,
    0xb7, 0x79, 0x94, 0x50, 0x17, 0x7b, 0x52, 0x6a, 0xb2, 0xad, 0x7d, 0x2e,
    0xa9, 0x9b, 0xa9, 0x56, 0xff, 0x0e, 0xa5, 0x1a, 0x49, 0x7a, 0x82, 0x9a,
    0xa4, 0x99, 0x84, 0x11, 0xeb, 0xf3, 0x57, 0x30, 0x6a, 0xfc, 0xc0, 0xd8,
    0x9f, 0x01, 0xe4, 0x35, 0xa7, 0xc9, 0xe1, 0xce, 0x5e, 0xce, 0xc4, 0xf4,
    0xf8, 0x92, 0x39, 0x8f, 0x73, 0x4c, 0x76, 0x41, 0x56, 0x5e, 0xa1, 0x33,
    0xdd, 0xeb, 0xca, 0xa7, 0x1e, 0x2c, 0xa9, 0x44, 0xc2, 0x49, 0x9d, 0xed,
    0x1c, 0xef, 0x9c, 0xa8, 0xc3, 0xe6, 0x39, 0x2c, 0xbc, 0x4a, 0xe5, 0xa0,
    0x2d, 0x32, 0xcf, 0x94, 0xbc, 0x2b, 0x4e, 0xc1, 0x92, 0x6d, 0x5a, 0xb7,
    0x79, 0x1c, 0x1d, 0x73, 0x6c, 0xa6, 0xfc, 0x69, 0xf8, 0xbc, 0x98, 0x03,
    0x23, 0x4e, 0x99, 0x1a, 0x9b, 0xa3, 0x21, 0x72, 0xfb, 0x05, 0x48, 0xdb,
    0x14, 0x73, 0xa2, 0x46, 0xe2, 0x36, 0x90, 0xcd, 0x38, 0xaa, 0x5d, 0x6b,
    0x27, 0xcc, 0x83, 0x90, 0xfe, 0x0d, 0xb9, 0x64, 0x92, 0x1a, 0xb1, 0x59,
    0xf6, 0x88, 0xd6, 0xfc, 0xeb, 0x97, 0x90, 0x77, 0xfc, 0xb9, 0xd4, 0xbd,
    0x3b, 0x92, 0xe7, 0x6c, 0x19, 0xd3, 0x95, 0xc1, 0x96, 0x89, 0xbf, 0x6b,
    0x5f, 0xd9, 0x80, 0xff, 0x07, 0xdd, 0x9c, 0x3b, 0xbc, 0x76, 0x6f, 0x32,
    0x13, 0x35, 0x0b, 0xa1, 0x17, 0x2a, 0x1e, 0x17, 0x77, 0x6e, 0x63, 0xc8,
    0x96, 0x0c, 0x35, 0x6f, 0xe3, 0xc5, 0x22, 0xa5, 0x75, 0xfe, 0xa3, 0x8c,
    0xa8, 0x41, 0x35, 0x15, 0x13, 0x3b, 0x5b, 0x42, 0x25, 0x3d, 0xb6, 0xec,
    0x7a, 0x86, 0x41, 0xcb, 0xe9, 0x0c, 0xa7, 0x7e, 0xd8, 0x97, 0x26, 0xb5,
    0x52, 0x91, 0x6f, 0xce, 0x71, 0x3b, 0xbe, 0x0f, 0x51, 0xad, 0xa0, 0x12,
    0xb6, 0xdd, 0x22, 0x9d, 0x19, 0xd1, 0x32, 0xc4, 0x8d, 0x1e, 0x67, 0x37,
    0x3b, 0x1a, 0x7c, 0x05, 0xdb, 0xd3, 0xd8, 0xe0, 0xac, 0x18, 0x74, 0xb3,
    0x95, 0xe6, 0xc3, 0x58, 0xf0, 0x82, 0x07, 0xfd, 0x7e, 0x45, 0x91, 0x8e,
    0x9b, 0x83, 0x45, 0xa7, 0x3b, 0xe0, 0xde, 0xb2, 0x0d, 0x00, 0x5f, 0x79,
    0x56, 0x1f, 0x77, 0x5a, 0xf2, 0x0a, 0x87, 0x00, 0x68, 0x0f, 0x50, 0x0b,
    0x59, 0x2a, 0x16, 0x74, 0x23, 0xc5, 0x80, 0xed, 0xe8, 0x36, 0xd2, 0x13,
    0xb5, 0x67, 0xa1, 0xad, 0x77, 0xaf, 0xc4, 0x76, 0x34, 0xb3, 0x09, 0x93,
    0x43, 0x3f, 0x76, 0x7a, 0xe7, 0xc6, 0xbd, 0x5b, 0xac, 0x99, 0xf6, 0xe1,
    0x03, 0x4a, 0x0a, 0xe3, 0xcc, 0x89, 0x74, 0x39, 0x2f, 0x09, 0xbb, 0xe6,
    0x7c, 0x80, 0x38, 0x66, 0x06, 0xd6, 0xa9, 0x4f, 0xc2, 0x11, 0x0e, 0x9e,
    0xa1, 0x5d, 0x9a, 0x8d, 0xee, 0x08, 0x31, 0x9d, 0x8c, 0x25, 0x28, 0xdc,
    0x9f, 0x95, 0x60, 0x2b, 0x9c, 0x02, 0xca, 0x22, 0xd0, 0xe9, 0x39, 0x4f,
    0x8f, 0xe2, 0xd7, 0x92, 0x65, 0x65, 0x57, 0x60, 0xc7, 0x9d, 0xa7, 0x8b,
    0x01, 0x5e, 0x6e, 0x40, 0x2f, 0x77, 0xf3, 0x48, 0x27, 0x35, 0xb4, 0xe3,
    0x56, 0x7b, 0x69, 0x53, 0xa3, 0xfc, 0xdb, 0xe8, 0x94, 0x53, 0xc4, 0x70,
    0xda, 0x78, 0x24, 0xde, 0x3f, 0xdc, 0x21, 0x12, 0xea, 0x28, 0x13, 0x28,
    0x21, 0xd1, 0x6d, 0x18, 0x02, 0x47, 0x31, 0x6e, 0xf8, 0xf7, 0x45, 0x83,
    0x9f, 0x8d, 0x05, 0x1e, 0xf9, 0x4e, 0x52, 0x9e, 0x34, 0x5d, 0x3b, 0xa8,
    0x97, 0xcd, 0x30, 0xb9, 0x1e, 0x47, 0xdc, 0x2f, 0x5a, 0x22, 0x70, 0xb3,
    0x36, 0x16, 0xa6, 0x16, 0x52, 0xde, 0x96, 0xa1, 0xf5, 0x41, 0x04, 0x71,
    0x2a, 0xc4, 0x45, 0xf8, 0x8b, 0x42, 0x65, 0x5d, 0x85, 0x96, 0xb1, 0x14,
    0xff, 0x71, 0xe3, 0xe3, 0xb4, 0x4a, 0x46, 0x41, 0x24, 0x53, 0xfe, 0xa7,
    0xa6, 0xc6, 0x13, 0x22, 0x79, 0x31, 0xea, 0xfb, 0x8e, 0x8e, 0x41, 0x1f,
    0xbb, 0x26, 0x81, 0x44, 0x9a, 0xc8, 0x7e, 0x4f, 0xf5, 0x73, 0xce, 0x6c,
    0xea, 0x0a, 0x10, 0x36, 0xb4, 0x7c, 0x76, 0x2d, 0xbd, 0xf8, 0xd3, 0xf5,
    0x0c, 0x85, 0xce, 0xce, 0xf3, 0xe5, 0x9e, 0x37, 0xea, 0xee, 0x4d, 0x64,
    0xf4, 0xf3, 0x30, 0xa5, 0x85, 0x84, 0xd5, 0xe3, 0x30, 0x4b, 0xe9, 0xab,
    0x0e, 0x5f, 0x12, 0x59, 0x4c, 0xb6, 0x61, 0xfe, 0xce, 0x59, 0x3e, 0x9e,
    0xd5, 0x09, 0x1d, 0xb9, 0x3a, 0x6b, 0x7b, 0xc6, 0x2c, 0x4f, 0x6e, 0xdc,
    0x67, 0xaa, 0x37, 0xe0, 0xd6, 0x88, 0xc7, 0x31, 0xba, 0x64, 0x7b, 0xce,
    0xca, 0xf8, 0x69, 0x34, 0xd1, 0xcb, 0x28, 0xc3, 0x35, 0x29, 0x55, 0x54,
    0xe4, 0xa8, 0xf2, 0x49, 0x31, 0xc6, 0x2c, 0xa8, 0x39, 0x85, 0x76, 0x06,
    0xd6, 0x78, 0x96, 0x58, 0xc0, 0x09, 0x1a, 0xf7, 0xd1, 0xcc, 0xf0, 0x41,
    0xef, 0x4d, 0x8a, 0xf7, 0x62, 0x9e, 0x51, 0x79, 0xfb, 0xf9, 0x2b, 0x54,
    0xc6, 0xd4, 0x67, 0x83, 0x0b, 0x87, 0x38, 0xa8, 0x6c, 0xfd, 0xce, 0x55,
    0xa2, 0xa4, 0xa5, 0x2b, 0x32, 0x66, 0xb3, 0xed, 0xac, 0x7e, 0x9a, 0x43,
    0xd7, 0xfc, 0x28, 0xd5, 0x17, 0x0c, 0xab, 0xed, 0x34, 0x4d, 0x56, 0x4a,
    0x9b, 0xf8, 0x92, 0x1b, 0x5b, 0x83, 0x91, 0x69, 0x17, 0xcb, 0x46, 0xdb,
    0x4e, 0xa2, 0x9a, 0x16, 0x4b, 0xbb, 0xf7, 0xeb, 0x86, 0x16, 0x6b, 0x3f,
    0xac, 0x1e, 0xfd, 0x5e, 0x0e, 0xf5, 0x87, 0xa8, 0xc4, 0xf7, 0x98, 0x37,
    0x18, 0x67, 0x6d, 0xab, 0x26, 0x42, 0xa6, 0xca, 0x9a, 0xa6, 0x9d, 0x24,
    0xd1, 0x7f, 0x27, 0x46, 0x8b, 0x13, 0xc5, 0x4f, 0xb2, 0x8e, 0x37, 0xb7,
    0x8f, 0x50, 0xd0, 0xed, 0x8f, 0xf9, 0x1a, 0x75, 0x17, 0x6a, 0x23, 0x00,
    0x91, 0x53, 0x62, 0x75, 0xea, 0x35, 0x14, 0x0b, 0x01, 0x2f, 0x8b, 0x5d,
    0xbf, 0xe6, 0x4a, 0x56, 0xd7, 0x3d, 0xb0, 0x45, 0xa9, 0x74, 0x86, 0xc9,
    0xa4, 0x30, 0xa6, 0x83, 0xdb, 0xee, 0xc8, 0x0c, 0x95, 0x4f, 0xda, 0x5a,
    0xf1, 0x43, 0x02, 0xc4, 0x12, 0xdc, 0xb7, 0x79, 0x02, 0x4b, 0x5c, 0xb3,
    0xba, 0x8c, 0xe9, 0x15, 0xae, 0xab, 0x79, 0xd6, 0x9a, 0x2f, 0xde, 0xe7,
    0xeb, 0xfe, 0x8a, 0xe9,
};

/* Hex string of _data[] */
static const char _hexstr[2 * DATA_SIZE + 1] =
    "47408FE5EF302C72544A439BECB81C78D8E154CD463D2C8362950C85163B22E6"
    "78A45AAC3AE9037285ACE60F936078E0826BC196517434E81BE7D237BB3F0523"
    "9A47A5320F0A2EAA9FB6EA4C3BFAF987F0BED3DCF504C14A4B038E4E2BFB6466"
    "3EED2B3C860946C6B24071DDA15A7DE052A591A5ABE78700513E5A799415E727"
    "7F730A017F8B32A8B23288D90F675184D3D6916C18E7A81A9665E8BF06A1EFB7"
    "9FCAE2F150C6E24616C0695BA6361C0F0E4FC81349124728E1CC3B996637FCF8"
    "3F4F48E9C48E5532FAC324E6B7799450177B526AB2AD7D2EA99BA956FF0EA51A"
    "497A829AA4998411EBF357306AFCC0D89F01E435A7C9E1CE5ECEC4F4F892398F"
    "734C7641565EA133DDEBCAA71E2CA944C2499DED1CEF9CA8C3E6392CBC4AE5A0"
    "2D32CF94BC2B4EC1926D5AB7791C1D736CA6FC69F8BC9803234E991A9BA32172"
    "FB0548DB1473A246E23690CD38AA5D6B27CC8390FE0DB964921AB159F688D6FC"
    "EB979077FCB9D4BD3B92E76C19D395C19689BF6B5FD980FF07DD9C3BBC766F32"
    "13350BA1172A1E17776E63C8960C356FE3C522A575FEA38CA8413515133B5B42"
    "253DB6EC7A8641CBE90CA77ED89726B552916FCE713BBE0F51ADA012B6DD229D"
    "19D132C48D1E67373B1A7C05DBD3D8E0AC1874B395E6C358F08207FD7E45918E"
    "9B8345A73BE0DEB20D005F79561F775AF20A8700680F500B592A167423C580ED"
    "E836D213B567A1AD77AFC47634B30993433F767AE7C6BD5BAC99F6E1034A0AE3"
    "CC8974392F09BBE67C80386606D6A94FC2110E9EA15D9A8DEE08319D8C2528DC"
    "9F95602B9C02CA22D0E9394F8FE2D79265655760C79DA78B015E6E402F77F348"
    "2735B4E3567B6953A3FCDBE89453C470DA7824DE3FDC2112EA28132821D16D18"
    "0247316EF8F745839F8D051EF94E529E345D3BA897CD30B91E47DC2F5A2270B3"
    "3616A61652DE96A1F54104712AC445F88B42655D8596B114FF71E3E3B44A4641"
    "2453FEA7A6C613227931EAFB8E8E411FBB2681449AC87E4FF573CE6CEA0A1036"
    "B47C762DBDF8D3F50C85CECEF3E59E37EAEE4D64F4F330A58584D5E3304BE9AB"
    "0E5F12594CB661FECE593E9ED5091DB93A6B7BC62C4F6EDC67AA37E0D688C731"
    "BA647BCECAF86934D1CB28C335295554E4A8F24931C62CA839857606D6789658"
    "C0091AF7D1CCF041EF4D8AF7629E5179FBF92B54C6D467830B8738A86CFDCE55"
    "A2A4A52B3266B3EDAC7E9A43D7FC28D5170CABED344D564A9BF8921B5B839169"
    "17CB46DB4EA29A164BBBF7EB86166B3FAC1EFD5E0EF587A8C4F7983718676DAB"
    "2642A6CA9AA69D24D17F27468B13C54FB28E37B78F50D0ED8FF91A75176A2300"
    "91536275EA35140B012F8B5DBFE64A56D73DB045A97486C9A430A683DBEEC80C"
    "954FDA5AF14302C412DCB779024B5CB3BA8CE915AEAB79D69A2FDEE7EBFE8AE9";

int main(int argc, const char* argv[])
{
    oe_result_t result;
    oe_enclave_t* enclave = NULL;

    /* Check command-line argument count */
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s ENCLAVE_PATH\n", argv[0]);
        return 1;
    }

    /* Create the enclave */
    const uint32_t flags = oe_get_create_flags();
    OE_TEST(
        (result = oe_create_enclave(
             argv[1], OE_ENCLAVE_TYPE_SGX, flags, NULL, 0, &enclave)) == OE_OK);

    /* Check buffer sizes */
    {
        Args args;
        OE_TEST(sizeof(args.data) == sizeof(_data));
        OE_TEST(sizeof(args.hexstr) == sizeof(_hexstr));
    }

    /* Test enclave version of oe_hex_string() */
    {
        Args args;
        memcpy(args.data, _data, sizeof(args.data));
        args.ret = -1;

        OE_TEST((result = oe_call_enclave(enclave, "Test", &args)) == OE_OK);
        OE_TEST(args.ret == 0);
        OE_TEST(strcmp(args.hexstr, _hexstr) == 0);
    }

    /* Test host version of oe_hex_string() */
    {
        unsigned char data[DATA_SIZE];
        char hexstr[2 * DATA_SIZE + 1];
        memcpy(data, _data, sizeof(data));

        OE_TEST(oe_hex_string(hexstr, sizeof(hexstr), data, sizeof(data)));
        OE_TEST(strcmp(hexstr, _hexstr) == 0);

        oe_hex_dump(_data, DATA_SIZE);
    }

    /* Terminate the test enclave */
    oe_terminate_enclave(enclave);

    printf("=== passed all tests (hexdump)\n");

    return 0;
}

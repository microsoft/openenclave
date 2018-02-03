#include "../common/heap.c"

static void _DumpVAD(const OE_VAD* vad)
{
    size_t pages = vad->size / OE_PAGE_SIZE;
    printf("    OE_VAD{addr=%lx, end=%lx size=%u pages=%zu}\n", 
        vad->addr, vad->addr + vad->size, vad->size, pages);
}

static void _PrintGap(
    uintptr_t start,
    uintptr_t end)
{
    size_t size = end - start;
    size_t pages = size / OE_PAGE_SIZE;
    printf("    ......{addr=%lx, end=%lx size=%zu pages=%zu}\n", 
        start, end, size, pages);
}

void OE_HeapDump(const OE_Heap* h, bool full)
{
    const OE_VAD* p;

    uintptr_t base = h->base;

    printf("=== OE_Heap()\n");

    if (full)
    {
        printf("initialized:        %s\n", h->initialized ? "true" : "false");

        printf("size:               %lu\n", h->size);

        printf("num_pages:          %lu\n", (h->end - base) / OE_PAGE_SIZE);

        printf("num_vads:           %lu\n", h->end_vad - (OE_VAD*)base);

        printf("base:               %lx (0)\n", base);

        printf("next_vad:           %lx (%lu)\n", 
            (uintptr_t)h->next_vad, (uintptr_t)h->next_vad - base);

        printf("end_vad:            %lx (%lu)\n", 
            (uintptr_t)h->end_vad, (uintptr_t)h->end_vad - base);

        printf("start:              %lx (%lu)\n", h->start, h->start - base);

        printf("brk:          %lx (%lu)\n", 
            h->brk, h->brk - base);

        printf("map:         %lx (%lu)\n", 
            h->map, h->map - base);

        printf("end:                %lx (%lu)\n", h->end, h->end - base);
    }

    /* Dump the free VAD list */
    if (full)
    {
        printf("free_vads:\n");

        for (p = h->free_vads; p; p = p->next)
            _DumpVAD(p);
    }

    /* Dump the VAD list */
    {
        const OE_VAD* prev = NULL;

        printf("vad_list:\n");

        for (p = h->vad_list; p; p = p->next)
        {
            if (!prev)
            {
                if (h->map != p->addr)
                {
                    _PrintGap(h->map, p->addr);
                }
            }
            else if (prev && prev->addr + prev->size != p->addr)
            {
                _PrintGap(prev->addr + prev->size, p->addr);
            }

            _DumpVAD(p);

            prev = p;
        }

        if (prev && prev->addr + prev->size != h->end)
        {
            _PrintGap(prev->addr + prev->size, h->end);
        }
    }
}

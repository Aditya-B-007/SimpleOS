#define PAGE_STATE_FREE      0x01
#define PAGE_STATE_USED      0x02
#define PAGE_STATE_RESERVED  0x04
#define PAGE_STATE_KERNEL    0x08
typedef struct page_frame {
    uint8_t state;
    uint8_t order;
    struct page_frame* next;
    struct page_frame* prev;

} page_frame_t;
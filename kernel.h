#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

// ==========================================
// 1. IO.H (Hardware Port I/O)
// ==========================================
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void outw(uint16_t port, uint16_t val) {
    __asm__ __volatile__("outw %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ __volatile__("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void outl(uint16_t port, uint32_t val) {
    __asm__ __volatile__("outl %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ __volatile__("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// ==========================================
// 2. GRAPHICS.H (VBE & Framebuffer)
// ==========================================
typedef struct {
    uint16_t attributes;
    uint8_t  windowA, windowB;
    uint16_t granularity;
    uint16_t windowSize;
    uint16_t windowASegment, windowBSegment;
    uint32_t winFuncPtr;
    uint16_t pitch;
    uint16_t resolution_x, resolution_y;
    uint8_t  wChar, yChar, planes, bitsPerPixel, banks;
    uint8_t  memoryModel, bankSize, imagePages;
    uint8_t  reserved1;
    uint8_t  redMask, redMaskPosition;
    uint8_t  greenMask, greenMaskPosition;
    uint8_t  blueMask, blueMaskPosition;
    uint8_t  rsvdMask, rsvdMaskPosition;
    uint8_t  directColorAttributes;
    uint32_t physbase;
    uint32_t reserved2;
    uint16_t reserved3;
} __attribute__((packed)) VbeModeInfo;

extern VbeModeInfo vbe_mode_info;

typedef struct{
    void* address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bitsPerPixel;
    uint8_t bytesPerPixel;
} FrameBuffer;

typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t* data; 
} Bitmap;

typedef struct {
    uint32_t char_width;
    uint32_t char_height;
    const uint8_t (*bitmap)[16]; 
} Font;

void clear_screen(FrameBuffer* fb, uint32_t color);
void put_pixel(FrameBuffer* fb, int32_t x, int32_t y, uint32_t color);
void draw_line(FrameBuffer* fb, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);
void draw_rectangle(FrameBuffer* fb, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color);
void fill_rectangle(FrameBuffer* fb, int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color);
void draw_circle(FrameBuffer* fb, int32_t x, int32_t y, int32_t radius, uint32_t color);
void draw_bitmap(FrameBuffer* fb, Bitmap* bmp, int32_t x, int32_t y, uint32_t color);
void draw_char(FrameBuffer* fb, Font* font, char c, int32_t x, int32_t y, uint32_t color);
void draw_string(FrameBuffer* fb, Font* font, const char* str, int32_t x, int32_t y, uint32_t color);

// ==========================================
// 3. SYNC.H (Spinlocks)
// ==========================================
#ifndef __ATOMIC_ACQUIRE
#define __ATOMIC_ACQUIRE 2
#endif
#ifndef __ATOMIC_RELEASE
#define __ATOMIC_RELEASE 3
#endif

typedef struct {
    volatile bool locked;
} spinlock_t;

static inline void spinlock_init(spinlock_t* lock) {
    lock->locked = 0;
}

static inline void spinlock_acquire(spinlock_t* lock) {
    while (__atomic_test_and_set(&lock->locked, __ATOMIC_ACQUIRE)) {
        // "pause" instruction hints the CPU that this is a spin-wait loop.
        // It improves performance and power consumption on x86 processors.
        __asm__ __volatile__("pause");
    }
}

static inline void spinlock_release(spinlock_t* lock) {
    __atomic_clear(&lock->locked, __ATOMIC_RELEASE);
}

static inline unsigned long spinlock_acquire_irqsave(spinlock_t* lock) {
    unsigned long flags;
    __asm__ __volatile__("pushf; pop %0; cli" : "=r"(flags));
    spinlock_acquire(lock);
    return flags;
}

static inline void spinlock_release_irqrestore(spinlock_t* lock, unsigned long flags) {
    spinlock_release(lock);
    __asm__ __volatile__("push %0; popf" :: "r"(flags));
}

// ==========================================
// 4. VGA.H (Text Mode)
// ==========================================
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

void vga_init(void);
void vga_clear(void);
uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg);
void vga_setcolor(uint8_t color);
void vga_putentryat(char c, uint8_t color, size_t x, size_t y);
void vga_putchar(char c);
void vga_print_string(const char* str);
void vga_scroll(void);
void vga_print_hex(uint32_t n);
void vga_print_dec(uint32_t n);

// ==========================================
// 5. FONT.H
// ==========================================
extern const uint8_t font[256][16];

// ==========================================
// 6. GDT.H & IDT.H (CPU Tables)
// ==========================================
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void gdt_install(void);
void gdt_flush(void);

struct idt_entry {
    uint16_t base_lo;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_hi;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

typedef struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;

typedef void (*isr_t)(registers_t *);
void idt_install(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void irq_handler(registers_t *r);
void isr_handler(registers_t *r);
void irq_install_handler(int irq, void (*handler)(registers_t *r));
void page_fault_handler(registers_t *r);
void register_interrupt_handler(uint8_t n, isr_t handler);

// ISR Externs
extern void isr0(void); extern void isr1(void); extern void isr2(void); extern void isr3(void);
extern void isr4(void); extern void isr5(void); extern void isr6(void); extern void isr7(void);
extern void isr8(void); extern void isr9(void); extern void isr10(void); extern void isr11(void);
extern void isr12(void); extern void isr13(void); extern void isr14(void); extern void isr15(void);
extern void isr16(void); extern void isr17(void); extern void isr18(void); extern void isr19(void);
extern void isr20(void); extern void isr21(void); extern void isr22(void); extern void isr23(void);
extern void isr24(void); extern void isr25(void); extern void isr26(void); extern void isr27(void);
extern void isr28(void); extern void isr29(void); extern void isr30(void); extern void isr31(void);
extern void isr128(void);
extern void irq0(void); extern void irq1(void); extern void irq2(void); extern void irq3(void);
extern void irq4(void); extern void irq5(void); extern void irq6(void); extern void irq7(void);
extern void irq8(void); extern void irq9(void); extern void irq10(void); extern void irq11(void);
extern void irq12(void); extern void irq13(void); extern void irq14(void); extern void irq15(void);

// ==========================================
// 7. PMM.H (Physical Memory Manager)
// ==========================================
#define PAGE_SIZE 4096

typedef enum { PAGE_STATE_FREE=0, PAGE_STATE_USED=1, PAGE_STATE_RESERVED=2 } page_state_t;

typedef struct page_frame {
    struct page_frame* next;
    struct page_frame* prev;
    uint8_t order;
    page_state_t state;
} page_frame_t;

void pmm_init(uint32_t memory_end);
void* pmm_alloc_page(void);
void* pmm_alloc_pages(uint32_t count);
void pmm_free_page(void* p);
uint32_t pmm_get_free_memory(void);

// ==========================================
// 8. HEAP.H (Kernel Heap)
// ==========================================
void heap_init(uint32_t start_address, uint32_t size);
void* kmalloc(size_t size);
void kfree(void* ptr);

// ==========================================
// 9. PAGING.H (Virtual Memory)
// ==========================================
typedef struct page {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t unused     : 7;
    uint32_t frame      : 20;
} page_t;

typedef struct page_table {
    page_t pages[1024];
} page_table_t;

typedef struct page_directory {
    page_table_t *tables[1024];
    uint32_t physical_tables[1024];
    uint32_t physicalAddr;
} page_directory_t;

void paging_install(void);
void paging_map(uint32_t phys, uint32_t virt, uint32_t flags);
void switch_page_directory(page_directory_t *dir);
extern page_directory_t* page_directory;

// ==========================================
// 10. TIMER.H
// ==========================================
void timer_install(void);
uint32_t get_ticks();

// ==========================================
// 11. KEYBOARD.H & MOUSE.H
// ==========================================
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

void keyboard_install(void);
void keyboard_handler(registers_t *r);
char keyboard_getchar(void);
void keyboard_wait_for_input(void);
extern unsigned char kbdus[128];

typedef struct {
    uint8_t flags;
    int8_t x_delta;
    int8_t y_delta;
} mouse_packet_t;

extern mouse_packet_t mouse_packet;
extern volatile int32_t mouse_x;
extern volatile int32_t mouse_y;
extern volatile uint8_t mouse_buttons;
void mouse_install(void);

// ==========================================
// 12. PCI.H & NIC.H
// ==========================================
void pci_scan(void);
uint32_t pci_read_config(uint16_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void rtl8139_init(void);

// ==========================================
// 13. CURSOR.H & DIRTY_RECT.H
// ==========================================
void cursor_init(void);
void cursor_update(FrameBuffer*fb,int x,int y);
void cursor_draw(FrameBuffer*fb,int x,int y);

#define MAX_DIRTY_RECTS 32
typedef struct {
    int32_t x, y, width, height;
} Rect;

void dirty_rect_init(void);
void dirty_rect_add(int x, int y, int width, int height);
const Rect* dirty_rect_get_all(int* count);

// ==========================================
// 14. WIDGET.H
// ==========================================
struct Widget;
typedef enum {
    WIDGET_LABEL, WIDGET_BUTTON, WIDGET_CHECKBOX, WIDGET_RADIOBUTTON,
    WIDGET_SCROLLBAR, WIDGET_LISTBOX, WIDGET_PANEL, WIDGET_TEXTBOX,
    WIDGET_TEXTAREA, WIDGET_COMBOBOX, WIDGET_TABLE, WIDGET_TREE,
    WIDGET_GRAPH, WIDGET_DIALOG, WIDGET_SPINNER, WIDGET_CANVAS,
    WIDGET_PROGRESSBAR, WIDGET_FILECHOOSER, WIDGET_COLORCHOOSER,
    WIDGET_FONTCHOOSER, WIDGET_SLIDER, WIDGET_MENU, WIDGET_TOGGLEBUTTON,
    WIDGET_RADIOGROUP, WIDGET_VOICEINPUT, WIDGET_WINDOW
} WidgetType;

typedef enum {
    EVENT_MOUSE_CLICK = 1,
    EVENT_MOUSE_RELEASE = 2,
    EVENT_MOUSE_HOVER = 3,
    EVENT_MOUSE_MOVE = 4,
} event_type_t;

typedef struct Widget{
    WidgetType type;
    int32_t x, y, width, height;
    void (*draw)(struct Widget* self, FrameBuffer* fb);
    void (*update)(struct Widget* self, FrameBuffer* fb);
    void (*onClick)(struct Widget* self, int32_t mouse_x, int32_t mouse_y, int button);
    void (*onHover)(struct Widget* self, int32_t mouse_x, int32_t mouse_y);
    void (*onRelease)(struct Widget* self, int32_t mouse_x, int32_t mouse_y, int button);
    void (*onMove)(struct Widget* self, int32_t mouse_x, int32_t mouse_y);
    void* data;
    struct Widget* next;
} Widget;

typedef struct{
    char* text;
    uint32_t color;
} LabelData;

typedef struct{
    char* text;
    uint32_t color;
    uint32_t bg_color;
    uint32_t border_color;
    int border_width;
    uint32_t base_color;
    int base_width;
    uint32_t text_color;
    uint32_t press_color;
    uint32_t press_border;
    uint32_t hover_color;
    uint32_t hover_border;
} ButtonData;

void init_widget_system(void);
void widget_set_font(Font* font);
void widget_draw_all(Widget* head, FrameBuffer* fb);
void widget_update_all(Widget* head, FrameBuffer* fb);
void widget_handle_event_all(Widget* head, int mouse_x, int mouse_y, int event);
void widget_add(Widget** head, Widget* new_widget);
void widget_remove(Widget** head, Widget* widget);
void widget_free(Widget* widget);
void widget_free_all(Widget** head);
void widget_update(Widget* widget, FrameBuffer* fb);
void widget_handle_event(Widget* widget, int mouse_x, int mouse_y, int event);
Widget* create_label(int x, int y, int width, int height, char* text, uint32_t color);
Widget* create_textbox(int x, int y, int width, int height, char* placeholder, uint32_t bg_color, uint32_t text_color);
Widget* create_scrollbar(int x, int y, int width, int height, uint32_t bg_color, uint32_t thumb_color);
Widget* create_button(int x, int y, int width, int height, char* text, uint32_t base_color, uint32_t hover_color, uint32_t press_color, uint32_t border_color, int border_width, uint32_t text_color, uint32_t press_border, uint32_t hover_border);
void widget_draw(Widget* widget, FrameBuffer* fb);

// ==========================================
// 15. WINDOW.H
// ==========================================
typedef struct Window {
    int32_t x, y, width, height;
    const char* title; 
    bool has_title_bar;
    Widget* child_widgets_head;
    Widget* child_widgets_tail;
    struct Window* next;
    struct Window* prev;
    bool close_button_hovered;
} Window;

void window_manager_init(void);
Window* create_window(int x,int y,int width,int height,const char* title,bool has_title_bar);
void window_add_widget(Window* window,Widget* widget);
void window_remove_widget(Window* window,Widget* widget);
void window_draw(Window* window,FrameBuffer* fb);
void window_update(Window* window,FrameBuffer* fb);
void window_on_click(Window* window,int mouse_x,int mouse_y,int button);
void window_on_hover(Window* window,int mouse_x,int mouse_y);
void window_on_release(Window* window,int mouse_x,int mouse_y,int button);
void window_on_move(Window* window,int mouse_x,int mouse_y);
void wm_process_mouse(int mouse_x,int mouse_y,uint8_t mouse_buttons,uint8_t last_buttons);
void window_destroy(Window** head, Window** tail, Window* win_to_destroy);
void window_bring_to_front(Window** head, Window** tail, Window* win);
void window_manager_handle_mouse(Window** head, Window** tail, int32_t mouse_x, int32_t mouse_y, uint8_t mouse_buttons, uint8_t last_buttons);
void wm_set_focus(Window* window);
void window_free_widgets(Window* window);
void window_handle_key(char key);
void window_free(Window* window);

// ==========================================
// 16. CONSOLE.H & SHELL.H
// ==========================================
void console_init(FrameBuffer* fb, Font* font);
void console_write(const char* str);
void console_write_dec(uint32_t n);

void shell_init(void);
void shell_handle_input(char ch);
void shell_print_prompt(void);

// ==========================================
// 17. SYSCALL.H & TASK.H
// ==========================================
void syscalls_install(void);
void syscall_handler(registers_t *r);

typedef enum {
    TASK_RUNNING, TASK_READY, TASK_SLEEPING, TASK_DEAD,
    TASK_BLOCKED, TASK_WAITING, TASK_KILLED, TASK_ZOMBIE
} task_state_t;

typedef struct task {
    int id;                 
    registers_t regs;       
    void* kernel_stack;    
    task_state_t state;     
    struct task* next;       
} task_t;

void tasking_install(void);
void create_task(char* name, void (*entry_point)(void));
void schedule(registers_t* r);
void schedule_and_release_lock(spinlock_t* lock, unsigned long flags);
task_t* get_current_task(void);

// ==========================================
// 18. STORAGE.H (ATA & File System)
// ==========================================
void ata_init(void);
void ata_read_sector(uint32_t lba, uint8_t* buffer);
void ata_write_sector(uint32_t lba, uint8_t* buffer);

typedef struct {
    char name[128];
    uint32_t size;
    uint32_t flags; // 0=File, 1=Directory
} fs_entry_t;

// ==========================================
// 19. SYSTEM.H (Panic & RTC)
// ==========================================
void panic(const char* message, const char* file, uint32_t line);
#define PANIC(msg) panic(msg, __FILE__, __LINE__)

void rtc_init(void);
void rtc_get_time(uint8_t* hour, uint8_t* minute, uint8_t* second);

// ==========================================
// 20. AUDIO.H
// ==========================================
void audio_init(void);
void audio_play(uint8_t* buffer, uint32_t size);
void audio_stop(void);

// ==========================================
// 21. MUTEX.H
// ==========================================
struct task;
typedef struct mutex {
    spinlock_t lock; // Spinlock in order to ensure that the atomic operations on the mutex itself are thread-safe
    bool locked;// This is for checking if the mutex is currrently being held or not
    struct task* owner;//The task that currently holds the lock
    struct task* wait_queue;//A linked list of tasks waiting for the lock.
} mutex_t;
//Function prototypes for the mutexes operations we would be defining
void mutex_init(mutex_t* mutex);
void mutex_lock(mutex_t* mutex);
void mutex_unlock(mutex_t* mutex);

#endif // KERNEL_H

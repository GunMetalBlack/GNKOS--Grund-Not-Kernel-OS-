#include "kernel.h"
#include "utils.h"
#include "char.h"

uint32 vga_index;
static uint32 next_line_index = 1;
uint8 g_fore_color = WHITE, g_back_color = BLUE;
int digit_ascii_codes[10] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};

/*
this is same as we did in our assembly code for vga_print_char

vga_print_char:
  mov di, word[VGA_INDEX]
  mov al, byte[VGA_CHAR]

  mov ah, byte[VGA_BACK_COLOR]
  sal ah, 4
  or ah, byte[VGA_FORE_COLOR]

  mov [es:di], ax

  ret

*/
uint16 vga_entry(unsigned char ch, uint8 fore_color, uint8 back_color) 
{
  uint16 ax = 0;
  uint8 ah = 0, al = 0;

  ah = back_color;
  ah <<= 4;
  ah |= fore_color;
  ax = ah;
  ax <<= 8;
  al = ch;
  ax |= al;

  return ax;
}

void clear_vga_buffer(uint16 **buffer, uint8 fore_color, uint8 back_color)
{
  uint32 i;
  for(i = 0; i < BUFSIZE; i++){
    (*buffer)[i] = vga_entry(NULL, fore_color, back_color);
  }
  next_line_index = 1;
  vga_index = 0;
}

void init_vga(uint8 fore_color, uint8 back_color)
{
  vga_buffer = (uint16*)VGA_ADDRESS;
  clear_vga_buffer(&vga_buffer, fore_color, back_color);
  g_fore_color = fore_color;
  g_back_color = back_color;
}

void print_new_line()
{
  if(next_line_index >= 55){
    next_line_index = 0;
    clear_vga_buffer(&vga_buffer, g_fore_color, g_back_color);
  }
  vga_index = 80*next_line_index;
  next_line_index++;
}

void print_char(char ch)
{
  vga_buffer[vga_index] = vga_entry(ch, g_fore_color, g_back_color);
  vga_index++;
}

void print_string(char *str)
{
  uint32 index = 0;
  while(str[index]){
    print_char(str[index]);
    index++;
  }
}

void print_int(int num)
{
  char str_num[digit_count(num)+1];
  itoa(num, str_num);
  print_string(str_num);
}

uint8 inb(uint16 port)
{
  uint8 ret;
  asm volatile("inb %1, %0" : "=a"(ret) : "d"(port));
  return ret;
}

void outb(uint16 port, uint8 data)
{
  asm volatile("outb %0, %1" : "=a"(data) : "d"(port));
}

char get_input_keycode()
{
  char ch = 0;
  while((ch = inb(KEYBOARD_PORT)) != 0){
    if(ch > 0)
      return ch;
  }
  return ch;
}

/*
keep the cpu busy for doing nothing(nop)
so that io port will not be processed by cpu
here timer can also be used, but lets do this in looping counter
*/
void wait_for_io(uint32 timer_count)
{
  while(1){
    asm volatile("nop");
    timer_count--;
    if(timer_count <= 0)
      break;
    }
}

void sleep(uint32 timer_count)
{
  wait_for_io(timer_count);
}

char test_input()
{
  char ch = 0;
  char keycode = 0;
  keycode = get_input_keycode();
  if(keycode == KEY_ENTER){
    print_new_line();
  }else{
    ch = get_ascii_char(keycode);
    return ch;
  }
  sleep(1000);
  return 0;
}


//Creating a fun data type
struct Entity {
    char graphic;
    int x_pos;
    int y_pos;
};

void build_map(struct Entity entities[], int e_size)
{
  char map[8][4] = {
    {'.', '.', '.', '.'},
    {'.', '.', '.', '.'},
    {'.', '.', '.', '.'},
    {'.', '.', '.', '.'},
    {'.', '.', '.', '.'},
    {'.', '.', '.', '.'},
    {'.', '.', '.', '.'},
    {'.', '.', '.', '.'}
  };
  for(int e = 0; e <e_size; e++)
  { 
    map[entities[e].x_pos][entities[e].y_pos] = entities[e].graphic;
  }
  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      print_char(map[i][j]);
    }
    print_new_line();
  }
}

void player_movement(char input, struct Entity entites[])
{
  if(input == 'A')
  {
    entites[0].y_pos = entites[0].y_pos - 1;
  }
  if(input == 'D')
  {
    entites[0].y_pos = entites[0].y_pos + 1;
  }
  if(input == 'W')
  {
    entites[0].x_pos = entites[0].x_pos - 1;
  }
  if(input == 'S')
  {
     entites[0].x_pos = entites[0].x_pos + 1;
  }
  if(entites[0].x_pos > 3)
  {
    entites[0].x_pos = 3;
  }
  if(entites[0].x_pos < 0)
  {
    entites[0].x_pos = 0;
  }
  if(entites[0].y_pos < 0)
  {
    entites[0].y_pos = 0;
  }
  if(entites[0].y_pos > 7)
  {
    entites[0].y_pos = 7;
  }
}

void kernel_entry()
{
  init_vga(WHITE, RED);
  struct Entity entities[1];
  //Entity [0] is always player
  entities[0] = (struct Entity){'P', 1, 1};
  entities[1] = (struct Entity){'O', 4, 2};
  char input = 'l'; 
  char last_input = 0;
  while (input != 0)
  {
    init_vga(WHITE, RED);
    print_string("Use W,A,S,D to move, Player x:");
    print_int(entities->x_pos);
    print_new_line();
    build_map(entities, 1);
    input = test_input();
    if(input != last_input)
    {
      player_movement(input, entities);
    }
    last_input = input;
  }
}


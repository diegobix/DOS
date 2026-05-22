
constexpr int BOOTINFO_MAGIC = 0xB007B007;

class diego {
  public:
    int edad = 0;
    diego(int edad) : edad(edad) {};
};

diego yo(28);

extern "C" void kernel_main()
{
  char *vga = reinterpret_cast<char *>(0xb8000);
  *vga = 'X';

  while (true);
}

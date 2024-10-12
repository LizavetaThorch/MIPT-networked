#include <vector>
#include <cstring>

class Bitstream
{
public:
  Bitstream()
  {
    bits = std::vector<char>();
  }

  template <typename Type>
  Bitstream(const Type *data, size_t size)
  {
    bits = std::vector<char>(size);
    memcpy(bits.data(), data, size);
  }

  template <typename Type>
  void write(const Type &val)
  {
    bits.resize(bits.size() + sizeof(Type));
    memcpy(bits.data() + pos, &val, sizeof(Type));
    pos += sizeof(Type);
  }

  template <typename Type>
  void read(Type &val)
  {
    memcpy(&val, bits.data() + pos, sizeof(Type));
    pos += sizeof(Type);
  }

  char *get()
  {
    return bits.data();
  }

  size_t size()
  {
    return bits.size();
  }

private:
  std::vector<char> bits;
  size_t pos = 0;
};

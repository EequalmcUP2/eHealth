
class bc85
{
public:
  void process(uint8_t *pData, size_t length);

private:
  float sfloat_to_float(uint8_t msb, uint8_t lsb);
};
int main() {
  int x;
  const int k = 10 + 11;
  int y = k;
  x = y + 1;
  const int n = k * 7;
  int z = n - x, w = n - y;
  w = w * 1 * 1 * 1;
  return z + w;
}
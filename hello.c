int main() {
  int a = 1;
  if (a > 1) {
    while (1);
  }  else {
    while (a < 10) {
      return a;
      a = a + 2;
    }
  }
  return a;
}
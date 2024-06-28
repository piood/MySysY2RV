int main() {
  int a = 1 || 2 && 3;
  int b = 0 && 1 || 0;
  int c = (1 && 0 || 1) * 4;
  int d = 5;
  const int e = 6 || 7 && 8;
  if (a == 1 || a == 2);
  if (b == 0 || b == 1) d = d + 1; else;
  if (a && b || c && d) d = d + e;
  return d || !c;
  return d || e;
}
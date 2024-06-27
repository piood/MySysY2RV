int main() {
  int a = 1, sum = 0;
  {
    a = a + 2;
    int b = a + 3;
    b = b + 4;
    sum = sum + a + b;
    {
      b = b + 5;
      int c = b + 6;
      a = a + c;
      sum = sum + a + b + c;
      {
        b = b + a;
        int a = c + 7;
        a = a + 8;
        sum = sum + a + b + c;
        {
          b = b + a;
          int b = c + 9;
          a = a + 10;
          const int a = 11;
          b = b + 12;
          sum = sum + a + b + c;
          {
            c = c + b;
            int c = b + 13;
            c = c + a;
            sum = sum + a + b + c;
          }
          sum = sum - c;
        }
        sum = sum - b;
      }
      sum = sum - a;
    }
  }
  return sum % 77;
}
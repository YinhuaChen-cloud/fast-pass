// 这个文件用来作为 突变点检测PASS 的测试用例

#include <stdbool.h>
#include <stdio.h>

typedef enum {
  INITIAL = 0,
  NEGOTIATED = 1,
  FULL_HANDSHAKE = 2
} s2n_handshake_type_flag;

// # Category Mutation point Mutate into
// 1 Unary Neg - Drop the operator
// 2 Not ! Drop the operator
void unaryMutate(int i1) {
  int o1 = -i1;
  bool o2 = !i1;
}

// Binary
// 3 Add + One of -, *, /, %
// 4 Sub - One of +, *, /, %
// 5 Mul * One of +, -, /, %
// 6 Div / One of +, -, *, %
// 7 Mod % One of +, -, *, /
void binaryMutate(int i1, int i2) {
  int o1 = i1 + i2;
  int o2 = i1 - i2;
  int o3 = i1 * i2;
  int o4 = i1 / i2;
  int o5 = i1 % i2;
  unsigned int o6 = (unsigned int)i1 / (unsigned int)i2;
  unsigned int o7 = (unsigned int)i1 % (unsigned int)i2;
}

// Bitwise
// 8 BitAnd & One of |, ˆ
// 9 BitOr | One of &, ˆ
// 10 BitXor ˆ One of &, |
// 11 Shl « One of »L, »A
// 12 LShr »L Shl «
// 13 AShr »A Shl «
void bitwiseMutate(int i1, int i2) {
  int o1 = i1 & i2;
  int o2 = i1 | i2;
  int o3 = i1 ^ i2;
  int o4 = i1 << i2;
  int o5 = (unsigned int)i1 >> i2;
  int o6 = i1 >> i2;
}

// Compare
// 14 Lt < One of <=, >=, >, ==, !=
// 15 Le <= One of <, >=, >, ==, !=
// 16 Ge >= One of <, <=, >, ==, !=
// 17 Gt > One of <, <=, >=, ==, !=
// 18 Equality Eq == !=
// 19 Neq != ==
void compareMutate(int i1, int i2) {
  int o1 = i1 < i2;
  int o2 = i1 <= i2;
  int o3 = i1 >= i2;
  int o4 = i1 > i2;
  int o5 = i1 == i2;
  int o6 = i1 != i2;
  unsigned int o7 = (unsigned int)i1 < (unsigned int)i2;
  unsigned int o8 = (unsigned int)i1 <= (unsigned int)i2;
  unsigned int o9 = (unsigned int)i1 >= (unsigned int)i2;
  unsigned int o10 = (unsigned int)i1 > (unsigned int)i2;
}

// Constant
// 20 <value> One of 0, 1, -1, MIN, MAX, etc.
// 21 <value> One of value+1, value-1, etc.
// 22 <value> A random value in range
int valueMutate() {
  return 1024;
}

// Structure
// 23 <if-else> Swap the branches
// 24 <continue> break the loop
// 25 <break> continue the loop
// 26 ITE ?: Swap the operands
int structureMutate(int i1, int i2, int i3) {
  if(i1 > i2) {
    i3++;
  }
  else {
    i3--;
  }
  for(int i = 0; i < 100; i++) {
    if(i < 50)
      continue;
    i3++;
    if(i3 > i2)
      break;
  }

  return i1 > i2 ? i3 : 0;
}

void callOther() {
  compareMutate(102, 103);
}

int main() {

  int value = valueMutate();
  printf("value = %d\n", value);

}





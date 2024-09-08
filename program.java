import java.util.*;
import java.util.Scanner;

class Program {
Scanner input = new Scanner(System.in);

public int fib(int n) {
  if (n <= 1) {
    return n;
  };
  return fib(n - 1) + fib(n - 2);
}

public static void main(String[] args) {
Program p = new Program();
  System.out.println(p.fib(25));
}
}


using System;

namespace HRealEngine
{
    public class Main
    {
        public Main()
        {
            Console.WriteLine("Main class constructor called.");
        }
        public void PrintHello()
        {
            Console.WriteLine("Hello, World from C#!");
        }
        public void PrintNumber(int number)
        {
            Console.WriteLine($"The number is: {number}");
        }
        public void PrintNumbers(int number1, int number2)
        {
            Console.WriteLine($"The numbers are: {number1} and {number2}");
        }
        public void PrintCustomMessage(string message)
        {
            Console.WriteLine(message);
        }
    }
}

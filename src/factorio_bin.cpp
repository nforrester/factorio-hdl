#include <iostream>

#include "Factorio.h"
#include "ArithmeticCombinator.h"
#include "DeciderCombinator.h"
#include "ConstantCombinator.h"
#include "Machine.h"

#include <unistd.h>

int main()
{
    usleep(100000);

    Factorio fac;

    SignalId iron = 0;
    SignalId copper = 1;
    SignalId sig_a = 2;
    SignalId sig_b = 3;

    auto & input = fac.new_entity<ConstantCombinator>();
    auto & adder = fac.new_entity<ArithmeticCombinator>(iron, ArithmeticCombinator::Op::ADD, copper, sig_a);
    auto & lesser = fac.new_entity<DeciderCombinator>(copper, DeciderCombinator::Op::LT, iron, sig_b, false);
    auto & output = fac.new_entity<Machine>();

    input.constants.add(iron, 10);
    input.constants.add(copper, 3);

    fac.connect(Wire::green, input.port, adder.in_port);
    fac.connect(Wire::red, output.port, adder.out_port);

    fac.connect(Wire::green, lesser.in_port, adder.in_port);
    fac.connect(Wire::red, lesser.out_port, adder.out_port);

    fac.build();

    std::cout << "Begin\n";

    std::cout << "Input:  " << fac.read(input.port) << "\n";
    std::cout << "Output: " << fac.read(output.port) << "\n";
    for (int i = 0; i < 5; ++i)
    {
        fac.tick();
        std::cout << "\nTick.\n";
        std::cout << "Input:  " << fac.read(input.port) << "\n";
        std::cout << "Output: " << fac.read(output.port) << "\n";
    }

    std::cout << "End\n";

    return 0;
}

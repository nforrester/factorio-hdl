#include <iostream>

#include "Factorio.h"
#include "entities/ArithmeticCombinator.h"
#include "entities/DeciderCombinator.h"
#include "entities/ConstantCombinator.h"
#include "entities/Machine.h"
#include "entities/Counter.h"
#include "entities/Hysteresis.h"

#include <unistd.h>

int main()
{
    usleep(100000);

    Factorio fac;

    auto & input = fac.new_entity<ConstantCombinator>();
    auto & adder = fac.new_entity<ArithmeticCombinator>(
        Signal::iron_plate,
        ArithmeticCombinator::Op::ADD,
        Signal::copper_plate,
        Signal::letter_a);
    auto & lesser = fac.new_entity<DeciderCombinator>(
        Signal::copper_plate,
        DeciderCombinator::Op::LT,
        Signal::iron_plate,
        Signal::letter_b,
        false);
    auto & output = fac.new_entity<Machine>();

    input.constants.add(Signal::iron_plate, 10);
    input.constants.add(Signal::copper_plate, 3);

    fac.connect(Wire::green, input.port, adder.in_port);
    fac.connect(Wire::red, output.port, adder.out_port);

    fac.connect(Wire::green, lesser.in_port, adder.in_port);
    fac.connect(Wire::red, lesser.out_port, adder.out_port);

    auto & tick_counter = fac.new_entity<ArithmeticCombinator>(
        Signal::letter_a,
        ArithmeticCombinator::Op::ADD,
        SignalValue(1),
        Signal::letter_a);
    fac.connect(Wire::green, tick_counter.in_port, tick_counter.out_port);

    auto & doubler = fac.new_entity<ArithmeticCombinator>(
        Signal::letter_a,
        ArithmeticCombinator::Op::MUL,
        SignalValue(2),
        Signal::letter_a);
    auto & dout = fac.new_entity<Machine>();
    fac.connect(Wire::green, tick_counter.out_port, doubler.in_port);
    fac.connect(Wire::green, doubler.out_port, dout.port);

    auto & c1 = fac.new_entity<Counter>(fac, Signal::letter_a, 1, Wire::green);
    auto & offset = fac.new_entity<ArithmeticCombinator>(
        Signal::letter_a,
        ArithmeticCombinator::Op::ADD,
        SignalValue(0),
        Signal::letter_a);
    auto & hyst = fac.new_entity<Hysteresis>(
        fac,
        Signal::letter_a,
        Signal::letter_b,
        50,
        10,
        Wire::green);
    auto & hout = fac.new_entity<Machine>();

    fac.connect(Wire::green, c1.port(), offset.in_port);
    fac.connect(Wire::green, offset.out_port, hyst.in_port());
    fac.connect(Wire::green, hout.port, hyst.out_port());

    fac.build();

    std::cout << "Begin\n";

    std::cout << "Input:  " << fac.read(input.port) << "\n";
    std::cout << "Output: " << fac.read(output.port) << "\n";
    std::cout << "Count:  " << fac.read(tick_counter.out_port) << "\n";
    std::cout << "Double: " << fac.read(dout.port) << "\n";

    std::cout << "Hyst in:  " << fac.read(hyst.in_port(), Wire::green) << "\n";
    std::cout << "Hyst out: " << fac.read(hout.port) << "\n";

    for (int i = 0; i < 5; ++i)
    {
        fac.tick();
        std::cout << "\nTick.\n";
        std::cout << "Input:  " << fac.read(input.port) << "\n";
        std::cout << "Output: " << fac.read(output.port) << "\n";
        std::cout << "Count:  " << fac.read(tick_counter.out_port) << "\n";
        std::cout << "Double: " << fac.read(dout.port) << "\n";

        std::cout << "Hyst in:  " << fac.read(hyst.in_port(), Wire::green) << "\n";
        std::cout << "Hyst out: " << fac.read(hout.port) << "\n";
    }

    for (int i = 0; i < 100; ++i)
    {
        fac.tick();
        std::cout << "\nTick.\n";
        std::cout << "Hyst in:  " << fac.read(hyst.in_port(), Wire::green) << "\n";
        std::cout << "Hyst out: " << fac.read(hout.port) << "\n";
    }

    std::cout << "End\n";

    return 0;
}

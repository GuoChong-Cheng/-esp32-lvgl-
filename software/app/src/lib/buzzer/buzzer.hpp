#pragma once

#include "lib/defs.h"

#include "melody.h"

class buzzer {
private:
    uint8_t m_pin;

public:
    buzzer(uint8_t pin) : m_pin(pin) {}

    /* tone */

    void channel(uint8_t channel = 0) { setToneChannel(channel); }

    void start(uint32_t frequency, uint32_t duration = 0) { tone(m_pin, frequency, duration); }

    void stop() { noTone(m_pin); }

    /* melody */

    void play(melody_t melody) { play(melody.melody, melody.size, melody.tempo); }

    void play(int* melody, int size, int tempo = 100);

    /* test */

    void test(void);
};

#if 0

#include "bsp/board_conf.h"
#include "lib/buzzer/buzzer.hpp"

#include <SimpleSerialShell.h>

buzzer buzzer(PIN_BUZZER);

// argv[0]: command
// argv[n]: params

int play_melody(int argc, char** argv)
{
    if (argc == 2) {
        switch (atoi(argv[1])) {
            case 0: buzzer.play(start_up); break;
            case 1: buzzer.play(message); break;
            case 2: buzzer.play(warring); break;
            case 3: buzzer.play(zelda_theme); break;       // 塞尔达传说
            case 4: buzzer.play(star_wars_theme); break;   // 星球大战
            case 5: buzzer.play(super_mario_bros); break;  // 超级马里奥
            case 6: buzzer.play(hedwigs_theme); break;     // 哈利波特
            case 7: buzzer.play(nokia_tune); break;        // 诺基亚
            case 8: buzzer.play(cannon_in_d); break;       // 卡农
            default: buzzer.test(); break;
        }
    }

    return EXIT_SUCCESS;
}

void setup()
{
    Serial.begin(115200);
    while (!Serial) {}

    shell.attach(Serial);
    shell.addCommand(F("play"), play_melody);

    // shell.execute("play 1");
}

void loop()
{
    shell.executeIfInput();
}

#endif
#include "buzzer.hpp"

void buzzer::play(int* melody, int size, int tempo)
{
    // notes of the moledy followed by the duration.
    // a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
    // !!negative numbers are used to represent dotted notes,
    // so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!

    // sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
    // there are two values per note (pitch and duration), so for each note there are four bytes
    int notes = size / sizeof(melody[0]) / 2;

    // this calculates the duration of a whole note in ms (60s/tempo)*4 beats
    int wholenote = (60000 * 4) / tempo;

    int divider = 0, noteDuration = 0;

    for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
        // calculates the duration of each note
        divider = melody[thisNote + 1];
        if (divider > 0) {
            // regular note, just proceed
            noteDuration = (wholenote) / divider;
        } else if (divider < 0) {
            // dotted notes are represented with negative durations!!
            noteDuration = (wholenote) / abs(divider);
            noteDuration *= 1.5;  // increases the duration in half for dotted notes
        }

        // we only play the note for 90% of the duration, leaving 10% as a pause
        start(melody[thisNote], noteDuration * 0.9);

        // Wait for the specief duration before playing the next note.
        delay_ms(noteDuration);

        // stop the waveform generation before the next note.
        stop();
    }
}

void buzzer::test(void)
{
    int f;
    for (f = 0; f < 2; f++) {
        start(1000);
        delay_ms(50);
        stop();
        delay_ms(20);
    }
    delay_ms(400);
    for (f = 100; f < 3000; f += 100) {
        start(f);
        delay_ms(20);
        stop();
    }
    for (f = 3000; f > 100; f -= 100) {
        start(f);
        delay_ms(20);
        stop();
    }
}

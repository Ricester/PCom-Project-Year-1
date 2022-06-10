# PCom-Project-Year-1
The project that I have made is a study timer, which activates using an RFID reader and both a specific NFC card and an NFC tag. There are also buttons connected to the breadboard that add or remove minutes from the timer, and after the timer has started, one of them acts as a pause/resume button. The OLED screen will then flash as the timer is paused, and leaves a small message on the screen when the timer is finished.

Inspiration

I was inspired to make something that would help me be more productive as I initially struggled to avoid procrastination in the early days of working on the project. Various ideas came to mind, but I wanted something that could be simple yet easily developed further. 

Initial Design Ideas

- I first started with the idea of just a regular timer that uses a button and counts down from thirty minutes, similar to a Pomodoro timer but without the five-minute break option.
- There were different iterations of the design, which included adding LEDs to show that the timer is active/inactive or adding a buzzer that makes a sound for the timer starting/ending.
- After various versions, I decided to add a small OLED screen that would display the timer which would be more beneficial to the user in terms of knowing how long was left on it. 
- To make the project more challenging for myself, I also added an NFC reader so that I could experiment with different ways that I could integrate it into the timer.

Further Improvements

- Further improvements that I could make to the project would be to turn it into a simple phone jail, with the study timer connected to it as a feature.
  - It would require a lock to be added, as well as a case for all of the cables so that it looks more organised than it currently is.
- I could also add a button for a break option so that, similar to a Pomodoro timer, there could be 5 minutes of an automatic break before the timer resumes.
- A more user-friendly UI could have been designed for the OLED screen also, rather than the initial “Please scan card” message, as well as maybe an animation for when the timer ends.

Equipment Used

- NFC reader + NFC card + NFC tag
- OLED screen
- 2x Buttons
- Soldering Iron

Solved Issues

- Originally when I added the buttons to the breadboard, they were very loose and therefore constantly had an interrupted circuit. 
  - In order to solve this, I soldered some jumper cables to the pins of the buttons and attached them to the breadboard via the cables.
- The buttons had a significant delay when pressing multiple times to add or remove minutes from the timer.
  - I reduced the button delay in milliseconds in the coding to account for this.

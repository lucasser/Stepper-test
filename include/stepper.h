/*
ALL UNITS IN MM!!!
Header file for everything to do with stepper motors:
    Custom stepper driver class that can convert distance to steps
    Axis class for controlling multiple steppers together
    supporting struct to pass coordinates for next move

*/

#ifndef Stepper_H
#define Stepper_H

#include "DRV8825.h"
#include "LinkedList.h"
#include <queue>
#include "ArduinoJson.h"
#include "sensors.h"
#include <vector>

using std::vector;


/*driver for individual motors
    driven using distance in millimeters as opposed to steps.
    time in seconds ([TODO]: remove this and include in func comment)
*/
// class Sttepper : public DRV8825{
//     public:

//     public:
//         //manual constructor
//         Stepper(int DIR, int STEPS, bool DIRECT = true);
//         //constructor using json object
//         Stepper(JsonVariant stepper);
//         //move to a coordinate
//         void moveAbsolute(float pos, float time);

//         //move a distance
//         void moveRelative(float dist, float time);

//         //set current position to zero
//         void zero();
//         //tick the motor (wait time is built in)
//         void tick();

//     private:
//         float maxPos; //max absolute position of motor
//         float stepLen = 0.2; //mm per step
//         int MOTORSTEPS = 200; //steps per revolution

//         float curPos = 0; //actual location of motor
//         float projPos = 0; //projected location of motor when the move queue gets evaluated

//         //used in tick function to start moves correctly
//         float delay = 0; //how long to wait for next move
//         float startTime; //when move started

//         std::queue<move> moveCommands; //queue of move commands to execute

//         void startNextMove(); //begins next move
//         int mmToSteps(float mm); //convert millimeter input into motor steps
// };

/*[TODO]:
    emergency shutdown
    level offset
    detailed comments on all functions (see any cpp func for reference)
*/
class Axis {
    //public functions
    public:
        //default constructor
        Axis();
        //json object constructor
        Axis(JsonVariant config);
        //destructor to delete sensor pointer
        ~Axis();

        /**
         * @brief configure all axis motors and sensors from a JSON config file
         * @param config JSON config file, see config/README.md
        */
        void loadConfig(JsonVariant config);
        //tick all motors
        void tick();
        //move all motors to coordinate
        void moveAbsolute(float pos, float time = 0);

        //move all motors a set distance
        void moveRelative(float pos, float time = 0);

        //wait for other axis to finish move
        void delay(float time);
        //level the axis using custom function
        void level(); //[TODO]: switch which axis, do leveling logic invoking sensors as needed

        //set absolute position of all motors in axis to zero.
        void zero();

    //helper functions
    private:
        //begins next move
        void startNextMove();
        //convert millimeter input into motor steps
        int mmToSteps(float mm);
        //convert motor step amount input into mm distance
        float stepsToMM(float mm);
        //register motor using json object
        void setupMotor(JsonVariant stepper);
        //add correct sensor type for leveling
        void setupSensor(JsonVariant sensor);
        float suspendPos;

        /** @brief stops all movement
         *
         * Saves current position and stashes move queue.
         * Movement can be resumed from that position with the move queue, or move queue can be trashed.
         * if moveAbsolute or moveRelative is called without a resume, move queue is automatically trashed
         */
        void stop();
        /// @brief resumes movement after a stop. If called without a stops returns void.
        /// @param restart if true, trashes move queue, if false, movement is resumed from where the stop was called.
        void resume(bool restart);

    //public variables
    public:
        //is the axis configured
        bool init = false;
        //pointer to sensor
        Sensor* levelSensor;

    //internal state variables
    private:
        //stores a stepper driver object allong with position data.
        struct Stepper {
            float curPos; //location of motor
            int MOTORSTEPS = 200; //steps per revolution
            int direction = 1; //1 for forward, -1 for reverse
            BasicStepperDriver* motor;
        };
        //stores move action data
        struct move {
            float dist;
            float time;
        };

        //set through config//
        float maxPos; //max absolute position of motor
        float stepLen = 0.2; //mm per step
        float offset = 0; //difference between sensor trigger and axis 0 location

        //keep track of dynamic axis state
        float curPos = 0; //actual location of axis
        float projPos = 0; //projected location of axis when the move queue gets evaluated
        int microstep = 1;

        //used in tick function to start moves correctly
        float delay = 0; //how long to wait for next move
        float startTime; //when move started

        //list of motors
        vector<Stepper> motors = vector<Stepper>();

        std::queue<move> moveCommands; //queue of move commands to execute

        //Suspend data//
        //queue of move commands stored during suspend
        std::queue<move> suspendedMoves = {};
        //position of axis when suspend was called
};

//contains 4 coordinates for 4 axis, and time for the move. Absolute positions only
struct moveCommand{
    std::array<float, 4> coords = {NAN, NAN, NAN, NAN}; //x, y, z, e. Add values for more axis
    float time;

    String toString() {
        String out = "time: ";
        out += time;
        out += ", coords: ";
        for (float x : coords) {
            out += x;
            out += ", ";
        }
        return out;
    }
};

#endif //stepper
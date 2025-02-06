// buttons and encoders
struct Encoder {
  int pinA;
  int pinB;

  Encoder(int a, int b) : pinA { a }, pinB { b } {
    pinMode(a, INPUT);
    pinMode(b, INPUT);
  }

  enum State {
    LOCK, // the encoder is fully into a position
    RIGHT_START, // the encoder is now turning right
    LEFT_START, // the encoder is now turning left
  } state = State::LOCK;

  int poll() {
    bool a = !digitalRead(pinA);
    bool b = !digitalRead(pinB);
    if (state == State::LOCK) {
      if (!a && b) {
        state = State::LEFT_START;
      }
      if (a && !b) {
        state = State::RIGHT_START;
      }
    }
    if (!a && !b) {
      if (state == State::RIGHT_START) {
        state = State::LOCK;
        return 1;
      }
      if (state == State::LEFT_START) {
        state = State::LOCK;
        return -1;
      }
    }
    return 0;
  }
};


struct PullupButton { // track button presses. HIGH = released, LOW = pressed.
  int pin;
  int lastState = 0;
  bool wasReleased = false;

  PullupButton(int p) : pin { p } {
    pinMode(p, INPUT_PULLUP);
  }

  void poll() {
    int v = digitalRead(pin);
    if (lastState == LOW && v == HIGH) { // if we just went from being pressed to being released
      wasReleased = true;
    }
    lastState = v;
  }

  bool wasReleasedSLC() { // slc = Since Last Check
    if (wasReleased) {
      wasReleased = false;
      return true;
    }
    return false;
  }
};
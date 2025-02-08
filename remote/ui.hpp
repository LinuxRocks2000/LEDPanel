// UI code

const int ENCODER_DRAG = 2; // ticks of the encoder to move to the next element

template <typename T> using UiCallback = void (*)(T);
using Display = U8G2_SSD1309_128X64_NONAME0_F_HW_I2C;


struct Box {
  int x;
  int y;
  int w;
  int h;

  static Box empty() {
    return Box { 0, 0, 0, 0 };
  }

  void extend(int e) {
    x -= e;
    y -= e;
    w += e * 2;
    h += e * 2;
  }
};


struct UIElement {
  virtual bool end() {
    return false;
  }

  virtual bool selectable() {
    return false;
  }

  virtual void render(Display* display);

  virtual bool interact() { // interact with a UI element. if it returns true, the UI element will *capture* the ui, and all interactions will be forwarded to it until it releasest he ui.
    return false;
  }

  virtual Box box() { // selectables should return a Box. this allows the main UI to render the tab border.
    return Box::empty();
  }

  /* --- capture functions --- */
  // when the interface is captured by a UIElement, all interactions are forwarded to these functions. if they ever return true, capture is reset.
  virtual bool capturedButton1() {
    return true;
  }

  virtual bool capturedButton2() {
    return true;
  }

  virtual bool capturedEncoder(int delta) { // because this is a relative encoder, the value is a CHANGE in position
    return true;
  }

  virtual const char* captureTooltip() { // return the tooltip for this capture (which will be displayed in the side box)
    return "";
  }
};

struct TextBanner : UIElement {
  int x;
  int y;
  const char* text;

  TextBanner(int _x, int _y, const char* _text) : x {_x}, y {_y}, text{_text} {}

  void render(Display* display) {
    display -> setDrawColor(1);
    display -> drawStr(x, y, text);
  }
};

struct EnableDisableToggleButton : UIElement {
  int x;
  int y;
  bool state = true;
  UiCallback<bool> onToggle;

  EnableDisableToggleButton(int _x, int _y, UiCallback<bool> _onToggle) : x { _x }, y { _y }, onToggle { _onToggle } {

  }

  Box box() {
    return Box {
      x, y, 31, 10
    };
  }

  bool selectable() {
    return true;
  }

  bool interact() {
    state = !state;
    onToggle(state);
    return false;
  }

  void render(Display* display) {
    if (state) {
      display -> setDrawColor(0); // clear the area behind
      display -> drawBox(x, y, 31, 10);
      display -> setDrawColor(1);
      display -> drawFrame(x, y, 31, 10);
    }
    else {
      display -> setDrawColor(1);
      display -> drawBox(x, y, 31, 10);
      display -> setDrawColor(0);
    }
    display -> drawStr(x + 2, y + 8, state ? "enabled" : "disabled");
  }
};

struct Slider : UIElement {
  int x;
  int y;
  UiCallback<int> onChange;
  int value = 128;

  Slider(int _x, int _y, UiCallback<int> _onChange) : x { _x }, y { _y }, onChange { _onChange } {

  }

  Box box() {
    return Box { x, y, 35, 6};
  }

  bool selectable() {
    return true;
  }

  void render(Display* display) { // todo: add something to indicate partial positions ('cause the slider is only 35 pixels and there are 256 values)
    display -> setDrawColor(0);
    display -> drawBox(x, y, 35, 6);
    display -> setDrawColor(1);
    display -> drawFrame(x, y, 25, 6);
    display -> drawBox(x, y, value * 35 / 255, 6);
  }

  bool interact() {
    return true;
  }

  const char* captureTooltip() {
    return "turn knob to change value\npress any button to confirm";
  }

  bool capturedButton1() {
    return true;
  }

  bool capturedButton2() {
    return true;
  }

  bool capturedEncoder(int delta) {
    value += delta * 5;
    if (value > 255) {
      value = 255;
    }
    if (value < 0) {
      value = 0;
    }
    onChange(value);
    return false;
  }
};


struct UI {
  static const int UI_ELEMENT_COUNT = 12;
  int uiSelected = 0;
  int selectableCount = 0;

  Display display;
  UIElement* elements[UI_ELEMENT_COUNT];
  int encoderPos = 0;
  int interactLock = -1; // -1 = not locked, anything else = an index in elements[]

  UI(
      UiCallback<bool> whenFrontToggle,
      UiCallback<bool> whenBackToggle,
      UiCallback<int> whenBackBrightnessChange,
      UiCallback<int> whenBackWarmthChange,
      UiCallback<int> whenFrontBrightnessChange,
      UiCallback<int> whenFrontWarmthChange) : display(U8G2_R0), elements {
        new TextBanner(5, 5, "front"),
        new TextBanner(48, 5, "back"),
        new EnableDisableToggleButton(9, 14, whenFrontToggle),
        new EnableDisableToggleButton(51, 14, whenBackToggle),
        new TextBanner(5, 32, "bright"),
        new TextBanner(48, 32, "bright"),
        new TextBanner(5, 50, "warmth"),
        new TextBanner(48, 50, "warmth"),
        new Slider(4, 36, whenFrontBrightnessChange),
        new Slider(4, 54, whenFrontWarmthChange),
        new Slider(47, 36, whenBackBrightnessChange),
        new Slider(47, 54, whenBackWarmthChange),
      }
  {
    display.begin();
    display.setFont(u8g2_font_micro_mr);
    checkSelected();
    for (int i = 0; i < UI_ELEMENT_COUNT; i ++) {
      if (elements[i] -> selectable()) {
        selectableCount ++;
      }
    }
  }

  void checkSelected() {
    int sel = (encoderPos / ENCODER_DRAG) % selectableCount;
    for (int i = 0; i < UI_ELEMENT_COUNT; i ++) {
      if (elements[i] -> selectable()) {
        if (sel == 0) {
          uiSelected = i;
          break;
        }
        sel --;
      }
    }
  }

  void render() {
    display.clearBuffer();
    for (int i = 0; i < UI_ELEMENT_COUNT; i ++) {
      elements[i] -> render(&display);
      if (i == uiSelected) {
        Box b = elements[i] -> box();
        b.extend(3);
        display.setDrawColor(1);
        display.drawFrame(b.x, b.y, b.w, b.h);
      }
    }
    display.sendBuffer();
  }

  void button1() {
    Serial.println("button 1");
    if (interactLock != -1) {
      if (elements[interactLock] -> capturedButton1()) {
        interactLock = -1;
      }
    }
    else {
      if (elements[uiSelected] -> interact()) { // call the interact routine and optionally lock the ui to that element
        interactLock = uiSelected;
      }
    }
    render();
  }

  void button2() { // when not captured, button 2 does nothing but force screen refresh
    Serial.println("button 2");
    if (interactLock != -1) {
      if (elements[interactLock] -> capturedButton2()) {
        interactLock = -1;
      }
    }
    render();
  }

  void encoderTurn(int delta) {
    Serial.print("encoder turned ");
    Serial.println(delta);
    if (interactLock != -1) {
      if (elements[interactLock] -> capturedEncoder(delta)) {
        interactLock = -1;
      }
    }
    else {
      encoderPos += delta;
      checkSelected();
    }
    render();
  }
};
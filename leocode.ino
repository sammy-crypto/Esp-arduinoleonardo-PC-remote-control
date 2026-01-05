#include <Keyboard.h>
#include <Mouse.h>

String line;
bool shiftOn = false;
bool ctrlOn  = false;
bool altOn   = false;

static int clamp127(int v) {
  if (v > 127) return 127;
  if (v < -127) return -127;
  return v;
}

void applyModPresses() {
  if (shiftOn) Keyboard.press(KEY_LEFT_SHIFT);
  if (ctrlOn)  Keyboard.press(KEY_LEFT_CTRL);
  if (altOn)   Keyboard.press(KEY_LEFT_ALT);
}
void applyModReleases() {
  if (shiftOn) Keyboard.release(KEY_LEFT_SHIFT);
  if (ctrlOn)  Keyboard.release(KEY_LEFT_CTRL);
  if (altOn)   Keyboard.release(KEY_LEFT_ALT);
}

void tapKey(uint8_t k) {
  applyModPresses();
  Keyboard.press(k);
  delay(5);
  Keyboard.release(k);
  applyModReleases();
}

void typeChar(char ch) {
  applyModPresses();
  Keyboard.write(ch);
  applyModReleases();
}

void typeString(const String& s) {
  applyModPresses();
  Keyboard.print(s);
  applyModReleases();
}

void tapWin() {
  Keyboard.press(KEY_LEFT_GUI);
  delay(5);
  Keyboard.release(KEY_LEFT_GUI);
}

void specialKey(const String& name) {
  // toggles
  if (name == "SHIFT") { shiftOn = !shiftOn; if (shiftOn) Keyboard.press(KEY_LEFT_SHIFT); else Keyboard.release(KEY_LEFT_SHIFT); return; }
  if (name == "CTRL")  { ctrlOn  = !ctrlOn;  if (ctrlOn)  Keyboard.press(KEY_LEFT_CTRL);  else Keyboard.release(KEY_LEFT_CTRL);  return; }
  if (name == "ALT")   { altOn   = !altOn;   if (altOn)   Keyboard.press(KEY_LEFT_ALT);   else Keyboard.release(KEY_LEFT_ALT);   return; }

  // momentary Win
  if (name == "WIN_TAP") { tapWin(); return; }

  // common keys
  if (name == "ENTER") { tapKey(KEY_RETURN); return; }
  if (name == "BKSP")  { tapKey(KEY_BACKSPACE); return; }
  if (name == "TAB")   { tapKey(KEY_TAB); return; }
  if (name == "ESC")   { tapKey(KEY_ESC); return; }
  if (name == "SPACE") { typeChar(' '); return; }
  if (name == "DEL")   { tapKey(KEY_DELETE); return; }

  // arrows
  if (name == "LEFT")  { tapKey(KEY_LEFT_ARROW); return; }
  if (name == "RIGHT") { tapKey(KEY_RIGHT_ARROW); return; }
  if (name == "UP")    { tapKey(KEY_UP_ARROW); return; }
  if (name == "DOWN")  { tapKey(KEY_DOWN_ARROW); return; }

  // function keys
  if (name == "F1")  { tapKey(KEY_F1); return; }
  if (name == "F2")  { tapKey(KEY_F2); return; }
  if (name == "F3")  { tapKey(KEY_F3); return; }
  if (name == "F4")  { tapKey(KEY_F4); return; }
  if (name == "F5")  { tapKey(KEY_F5); return; }
  if (name == "F6")  { tapKey(KEY_F6); return; }
  if (name == "F7")  { tapKey(KEY_F7); return; }
  if (name == "F8")  { tapKey(KEY_F8); return; }
  if (name == "F9")  { tapKey(KEY_F9); return; }
  if (name == "F10") { tapKey(KEY_F10); return; }
  if (name == "F11") { tapKey(KEY_F11); return; }
  if (name == "F12") { tapKey(KEY_F12); return; }
}

// --- Mouse handling ---
void handleMouseMove(const String& payload) {
  // payload: "dx,dy,scroll"
  int c1 = payload.indexOf(',');
  int c2 = payload.indexOf(',', c1 + 1);
  if (c1 < 0 || c2 < 0) return;

  int dx = payload.substring(0, c1).toInt();
  int dy = payload.substring(c1 + 1, c2).toInt();
  int sc = payload.substring(c2 + 1).toInt();

  dx = clamp127(dx);
  dy = clamp127(dy);
  sc = clamp127(sc);

  Mouse.move(dx, dy, sc);
}

void handleMouseButton(const String& payload) {
  // payload examples:
  // "LC" "RC" "MC" (click)
  // "LD" "LU" "RD" "RU" (down/up)
  if (payload == "LC") { Mouse.click(MOUSE_LEFT); return; }
  if (payload == "RC") { Mouse.click(MOUSE_RIGHT); return; }
  if (payload == "MC") { Mouse.click(MOUSE_MIDDLE); return; }

  if (payload == "LD") { Mouse.press(MOUSE_LEFT); return; }
  if (payload == "LU") { Mouse.release(MOUSE_LEFT); return; }
  if (payload == "RD") { Mouse.press(MOUSE_RIGHT); return; }
  if (payload == "RU") { Mouse.release(MOUSE_RIGHT); return; }
}

void handleCommand(const String& cmd) {
  if (cmd.length() < 3 || cmd.charAt(1) != ':') return;
  char type = cmd.charAt(0);
  String payload = cmd.substring(2);

  if (type == 'T') {
    if (payload.length() >= 1) typeChar(payload.charAt(0));
  } else if (type == 'P') {
    typeString(payload);
  } else if (type == 'S') {
    specialKey(payload);
  } else if (type == 'M') {
    handleMouseMove(payload);
  } else if (type == 'B') {
    handleMouseButton(payload);
  }
}

void setup() {
  Serial1.begin(115200);
  Keyboard.begin();
  Mouse.begin();
}

void loop() {
  while (Serial1.available()) {
    char c = (char)Serial1.read();
    if (c == '\n') {
      line.trim();
      if (line.length()) handleCommand(line);
      line = "";
    } else if (c != '\r') {
      line += c;
      if (line.length() > 350) line = "";
    }
  }
}

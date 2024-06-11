import controlP5.*;
import oscP5.*;
import netP5.*;

// Constructors for ControlP5 variables
ControlP5 VariableInput;
ControlP5 GoButton;
Button button;
Textfield var1;
Textfield var2;
Textfield var3;
Textfield var4;

// Initialize Variables
String val1;
String val2;
String val3;
String val4;

float amp;
int frq;
int mod;
int pat;
int initialTime;
int interval=300;
int play;

OscP5 osc;
NetAddress myIp;

void setup() {
  size(768,384);
  background(20);
  noStroke();
  fill(255);

  // Text which shows the labels and unit for what numbers you will be inputting
  textSize(20);
  text("Amplitude",width*.1,(height/2)-115);
  text("Frequency",width*.1,height/2-33);
  text("Modulation",width*.1,(height/2)+50);
  text("Hz",width*.35,height/2-33);
  text("Hz",width*.35,(height/2)+50);
  text("Pattern",width*.1,(height/2)+132);
  
  // Constructors for the class variables
  VariableInput = new ControlP5(this);
  GoButton = new ControlP5(this);
  osc = new OscP5(this, 6000);
  myIp = new NetAddress("127.0.0.1", 5040);
  
  // Create fonts
  PFont sans=createFont("SansSerif",22);
  PFont sansy = createFont("Verdana", 40);
  ControlFont font = new ControlFont(sansy);
  GoButton.setFont(font);

  // Creates textbox for Amplitude User Input
  var1=VariableInput.addTextfield("Amp. Input")
     .setPosition(width*.25,(height/2)-135)
     .setSize(60,25)
     .setFont(sans)
     .setFocus(false)
     .setColor(255)
     .setColorCursor(color(126,242,186))
     .setCaptionLabel("")
     //.setInputFilter((ControlP5.INTEGER))
     .setAutoClear(false)
     ;
     
  // Create textbox for Frequency User Input
  var2=VariableInput.addTextfield("Freq. Input")
     .setPosition(width*.25,(height/2)-52.5)
     .setSize(60,25)
     .setFont(sans)
     .setFocus(false)
     .setColor(255)
     .setColorCursor(color(126,242,186))
     .setCaptionLabel("")
     //.setInputFilter((ControlP5.INTEGER))
     .setAutoClear(false)
     ;
     
  // Creates textbox for Modulation User Input
  var3=VariableInput.addTextfield("Mod. Input")
     .setPosition(width*.25,(height/2)+30)
     .setSize(60,25)
     .setFont(sans)
     .setFocus(false)
     .setColor(255)
     .setColorCursor(color(126,242,186))
     .setCaptionLabel("")
     .setInputFilter((ControlP5.INTEGER))
     .setAutoClear(false)
     ;
     
  var4=VariableInput.addTextfield("Pat. Input")
      .setPosition(width*.25,(height/2)+112.5)
      .setSize(60,25)
      .setFont(sans)
      .setFocus(false)
      .setColor(255)
      .setColorCursor(color(126,242,186))
      .setCaptionLabel("")
      .setInputFilter((ControlP5.INTEGER))
      .setAutoClear(false)
      ;
     
  // Creates button for which starts the experiment
  button=GoButton.addButton("GO!")
     .setSwitch(true)
     .setPosition(width*.80-60,height/2-32.5)
     .setSize(120,65)
     .activateBy(ControlP5.RELEASE)
     ;
}

// Object which submits the textbox entries if the button is on
void pressbutton() {
  if (button.isOn()==true) {
      var1.submit();
      var2.submit();
      var3.submit();
      var4.submit();
  }
}

// This is an object which is automatically ran when a change to a controller occurs; in this case, 
// when the textfields have their info submitted, the inputs (as integers) are stored as the variables 
// amp, mod, frq, and pat
void controlEvent(ControlEvent theEvent) {
  if (theEvent.isAssignableFrom(Textfield.class)) {
    if (theEvent.getName().equals("Amp. Input")) {
      val1 = theEvent.getStringValue();
      amp=(float(val1));
      println(amp);
    }
    else if (theEvent.getName().equals("Freq. Input")) {
      val2 = theEvent.getStringValue();
      frq=(int(val2));
      println(frq);
    }
    else if (theEvent.getName().equals("Mod. Input")) {
      val3 = theEvent.getStringValue();
      mod=(int(val3));
      println(mod);
    }
    else if (theEvent.getName().equals("Pat. Input")) {
      val4 = theEvent.getStringValue();
      pat=(int(val4));
      println(pat);
    }
  }
}

void pressPlayMax() {
  OscMessage go = new OscMessage("play");
  osc.send(go,myIp);
}

void sendInputsToMax() {
  OscMessage freq = new OscMessage("frq");
  freq.add(frq);
  osc.send(freq,myIp);
  
  OscMessage ampl = new OscMessage("amp");
  ampl.add(amp);
  osc.send(ampl,myIp);
  
  OscMessage modu = new OscMessage("mod");
  modu.add(mod);
  osc.send(modu,myIp);
  
  OscMessage patt = new OscMessage("pat");
  patt.add(pat);
  osc.send(patt,myIp);
}

void draw() {
  // Checks if the button has been pressed, and, if so, submits what is currently in the textfields for
  // a specified time
  if (((keyPressed == true && key == TAB) || button.isMousePressed()) && millis() - initialTime >= interval) {
    initialTime = millis();
    button.setOn();
    pressbutton();
    sendInputsToMax();
    pressPlayMax();
  }

  else {button.setOff();}
}

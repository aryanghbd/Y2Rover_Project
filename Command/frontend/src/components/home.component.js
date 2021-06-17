import React from "react";

let styles = {
    bold: {fontWeight: 'bold'},
    italic: {fontStyle: 'italic'}
}

function Home() {

  return (
    <div className="App">
      <h2>Welcome to Group 15's Mars Rover!</h2>
      <img src={require('../image.jpg').default} height={400} width={500} />
      <h1> </h1>
      <h5 style={styles.bold}>In order to control the rover, go to the 'Add' page and enter the following instructions:</h5>
      <h6 style={styles.italic}>Instruction: FORWARD    Amount: 0-120 centimeters</h6>
      <h6 style={styles.italic}>Instruction: REVERSE    Amount: 0-120 centimeters</h6>
      <h6 style={styles.italic}>Instruction: TURN    Amount: 0-240</h6>
      <h6 style={styles.italic}>Instruction: POWERSAVE ON/OFF </h6>
      <h6 style={styles.italic}>Instruction: HELP</h6>
      <h6 style={styles.italic}>Instruction: STOP</h6>
      <h6 style={styles.italic}>Instruction: WAYPOINT    Amount: X,Y </h6>
      <h6 style={styles.italic}>Instruction: AUTOPILOT    Amount: X </h6>
    </div>
  );

  }
export default Home;

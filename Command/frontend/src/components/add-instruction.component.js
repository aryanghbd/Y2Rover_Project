import React, { Component } from "react";
import InstructionDataService from "../services/instruction.service";

var lastTopic
var lastMsg
var mqtt = require('mqtt')
var client = mqtt.connect("ws://18.117.97.5:9001", {clientId:"react_feedback", username:"hs2119", password:"marsrover"})
console.log("connected flag  " + client.connected);

var topic_list=["feedback", "error"];
console.log("subscribing to topics");
client.subscribe(topic_list,{qos:1}); //topic list

client.on("connect",function(){	
  console.log("connected  "+ client.connected);
  })
  

export default class AddInstruction extends Component {
  constructor(props) {
    super(props);
    this.onChangeInstruction = this.onChangeInstruction.bind(this);
    this.onChangeAmount = this.onChangeAmount.bind(this);
    this.saveInstruction = this.saveInstruction.bind(this);
    this.newInstruction = this.newInstruction.bind(this);
    this.newFeedback = this.newFeedback.bind(this);

    this.state = {
      currentInstruction: {
      id: null,
      instruction: "",
      amount: "", 
      status: false,
      
      submitted: false
      }
    };
  }

  componentDidMount() {
    this.newFeedback()
  }

  newFeedback() {
  //handle incoming messages
  client.on('message',function(topic, message, packet){
	//console.log("message is "+ message);
	//console.log("topic is "+ topic);
      alert(message)
      lastTopic = topic.toString()
      lastMsg = message
  })

  } 

  onChangeInstruction(e) {
    this.setState({
      instruction: e.target.value
    });
  }

  onChangeAmount(e) {
    this.setState({
      amount: e.target.value
    });
  }
  
  saveInstruction() {
    var data = {
      instruction: this.state.instruction,
      amount: this.state.amount
    };

    InstructionDataService.create(data)
      .then(response => {
        this.setState({
          id: response.data.id,
          instruction: response.data.instruction,
          amount: response.data.amount,
          status: response.data.status,

          submitted: true
        });
        console.log(response.data);
      })
      .catch(e => {
        console.log(e);
      });
  }

  newInstruction() {
    this.setState({
      id: null,
      instruction: "",
      amount: "",
      status: false,

      submitted: false
    });
  }

  render() {
    return (
      <div className="submit-form">
        {this.state.submitted ? (
          <div>
            <h4>You submitted successfully!</h4>
            <button className="btn btn-success" onClick={this.newInstruction}>
              Add
            </button>
          </div>
        ) : (
          <div>
            
            <div className="form-group">
              <label htmlFor="instruction">Instruction</label>
              <input
                type="text"
                className="form-control"
                id="instruction"
                required
                value={this.state.instruction}
                onChange={this.onChangeInstruction}
                name="instruction"
              />
            </div>  
        
            <div className="form-group">
              <label htmlFor="amount">Amount</label>
              <input
                type="text"
                className="form-control"
                id="amount"
                required
                value={this.state.amount}
                onChange={this.onChangeAmount}
                name="amount"
              />
            </div>

            <button onClick={this.saveInstruction} className="btn btn-success">
              Submit
            </button>
          </div>
        )}
      </div>
    );
  }
}
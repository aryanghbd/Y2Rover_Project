import React, { Component } from "react";
import InstructionDataService from "../services/instruction.service";

var lastMsg;
var mqtt = require('mqtt')
var client = mqtt.connect("ws://18.117.97.5:9001", {clientId:"react_active", username:"hs2119", password:"marsrover"})
console.log("connected flag  " + client.connected);

var topic_list=["active"];
console.log("subscribing to topics");
client.subscribe(topic_list,{qos:1}); //topic list

client.on("connect",function(){	
  console.log("connected  "+ client.connected);
  })


export default class Instruction extends Component {
  constructor(props) {
    console.log("subscribing to topics fgbfgf");
    super(props);
    this.onChangeInstruction = this.onChangeInstruction.bind(this);
    this.onChangeAmount = this.onChangeAmount.bind(this);
    this.getInstruction = this.getInstruction.bind(this);
    this.updateStatus = this.updateStatus.bind(this);
    this.deleteInstruction = this.deleteInstruction.bind(this);

    this.state = {
      currentInstruction: {
        id: null,
        instruction: "",
        amount: "",
        status: false
      },
      message: ""
    };
  }

  componentDidMount() {
    this.getInstruction(this.props.match.params.id);
  }

  onChangeInstruction(e) {
    const instruction = e.target.value;

    this.setState(function(prevState) {
      return {
        currentInstruction: {
          ...prevState.currentInstruction,
          instruction: instruction
        }
      };
    });
  }

  onChangeAmount(e) {
    const amount = e.target.value;
    
    this.setState(prevState => ({
      currentInstruction: {
        ...prevState.currentInstruction,
        amount: amount
      }
    }));
  }

  getInstruction(id) {
    InstructionDataService.get(id)
      .then(response => {
        this.setState({
          currentInstruction: response.data
        });
        console.log(response.data);
      })
      .catch(e => {
        console.log(e);
      });
  }

  updateStatus() {
    client.on('message',function(topic, message, packet){
      console.log("message is "+ message);
      console.log("topic is "+ topic);
      lastMsg = message.toString()
    })
    
      console.log(this.state.currentInstruction.id)
      var data = {
      id: this.state.currentInstruction.id,
      instruction: this.state.currentInstruction.instruction,
      amount: this.state.currentInstruction.amount,
      status: lastMsg
    }

    InstructionDataService.update(this.state.currentInstruction.id, data)
      .then(response => {
        this.setState(prevState => ({
          currentInstruction: {
            ...prevState.currentInstruction,
            status: lastMsg
          }
        }));
        console.log(response.data);
      })
      .catch(e => {
        console.log(e);
      });
  
  
  }


  deleteInstruction() {    
    InstructionDataService.delete(this.state.currentInstruction.id)
      .then(response => {
        console.log(response.data);
        this.props.history.push('/instruction')
      })
      .catch(e => {
        console.log(e);
      });
  }

  render() {
    const { currentInstruction } = this.state;

    return (
      <div>
        {currentInstruction ? (
          <div className="edit-form">
            <h4>Instruction</h4>
            <form>
              <div className="form-group">
                <label htmlFor="instruction">Instruction</label>
                <input
                  type="text"
                  className="form-control"
                  id="instruction"
                  value={currentInstruction.instruction}
                  onChange={this.onChangeInstruction}
                />
              </div>
              <div className="form-group">
                <label htmlFor="amount">Amount</label>
                <input
                  type="text"
                  className="form-control"
                  id="amount"
                  value={currentInstruction.amount}
                  onChange={this.onChangeAmount}
                />
              </div>

              <div className="form-group">
                <label>
                  <strong>Status:</strong>
                </label>
                {currentInstruction.status ? "Complete" : "Pending"}
              </div>
            </form>

           
              <button
                className="m-3 btn btn-sm btn-primary"
                onClick={() => this.updateStatus()}
              >
                Update
              </button>
            

            <button
              className="m-3 btn btn-sm btn-danger"
              onClick={this.deleteInstruction}
            >
              Delete
            </button>
            <p>{this.state.message}</p>
          </div>
        ) : (
          <div>
            <br />
            <p>Please click on an Instruction...</p>
          </div>
        )}
      </div>
    );
  }
}
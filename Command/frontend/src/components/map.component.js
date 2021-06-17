import {CanvasJSChart} from 'canvasjs-react-charts'
import React, { Component } from 'react';

var data = []
var pos = ['']
var updateInterval = 10;

var lastMsg
var lastTopic
var mqtt = require('mqtt')
var client = mqtt.connect("ws://18.117.97.5:9001", {clientId:"react_position", username:"hs2119", password:"marsrover"})
console.log("connected flag  " + client.connected);

var topic_list=["position","colour", "objectproximity", "objectcoordinates"];
console.log("subscribing to topics");
client.subscribe(topic_list,{qos:1}); //topic list

client.on("connect",function(){	
  console.log("connected  "+ client.connected);
  })


class Map extends Component {
	constructor() {
		super();
		this.updateChart = this.updateChart.bind(this);

		this.state = {
			currentObstacle: {
			colour: "",
			objectcoordinates: "",
			objectproximity: "", 
			}
		  };
	}
	componentDidMount() {
		setInterval(this.updateChart, updateInterval);
	}

	updateChart() {
		client.on('message',function(topic, message, packet){
     // console.log("message is "+ message);
     // console.log("topic is "+ topic);
      lastMsg = message.toString()
	  lastTopic = topic.toString()
	
    })

    if (lastTopic == 'colour'){
		this.setState({
			colour: lastMsg
		})
		//console.log(this.colour)
		this.render();
	  }
	  if (lastTopic == 'objectproximity'){
		this.setState({
			objectproximity: lastMsg
		})
		this.render();
	  }
	  if (lastTopic == 'objectcoordinates'){
		this.setState({
			objectcoordinates: lastMsg
		})
		this.render();
  }
	if (lastTopic == 'position'){
    	if (lastMsg != null && lastMsg != pos[pos.length - 1] && this.chart != null && lastMsg != '-128,-128'){
    		pos.push(lastMsg)
      		//console.log("diff value"+ lastMsg + " " + pos[pos.length - 1])
      
      		var coords_str = lastMsg.split(',')
      		var x = parseFloat(coords_str[0]);
      		var y = parseFloat(coords_str[1]);
      		data.push({x: x,y: y});
      		console.log(data)
      		this.chart.render();
    } }
	
	
		
	}


	render() {
		const options = {
			height: 600,
			width: 1300,
			title :{
				text: "Mars Rover Plot"
			},
			data: [{
				type: "line",
				dataPoints : data
			}]
		}

		return (
		<div>
			<CanvasJSChart options = {options}
				 onRef={ref => this.chart = ref}
			/>
			{/*VISION INTEGRATED*/}
			{/*<h4>Obstacle coloured: <div>{this.state.colour}</div> Has coordinates: <div>{this.state.objectcoordinates}</div> Distance from Rover: <div>{this.state.objectproximity}</div> </h4>*/}
		</div>
		);
	}
}
export default Map;                     
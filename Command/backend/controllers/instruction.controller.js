const db = require("../models");
const Instruction = db.instructions;

var mongo = require('mongodb')
var mongc = mongo.MongoClient

var mqtt = require('mqtt')
var client = mqtt.connect('mqtt://18.117.97.5', {clientId:"backend", username:"hs2119", password:"marsrover"})
console.log("connected flag  " + client.connected);

var topic_list=["position","colour", "objectproximity", "objectcoordinates"];
console.log("subscribing to topics");
client.subscribe(topic_list,{qos:1}); //topic list

function insertMessage(topic, message){
  mongc.connect("mongodb+srv://holly:marsrover@cluster0.mr177.mongodb.net/marsrover?retryWrites=true&w=majority", (error, client)=>{
      var myCol = client.db('marsrover').collection(topic.toString())
      myCol.insertOne({
          message: message.toString()
      }, ()=>{
          console.log('Data is saved to MongoDB')
          client.close()
      })
  })
}

var pos = ['']
//handle incoming messages
client.on('message',function(topic, message, packet){
//	console.log("message is "+ message);
//	console.log("topic is "+ topic);
  if (topic == 'position'){
    if (message.toString() != pos[pos.length - 1]){
      pos.push(message)
    //  insertMessage(topic, message)
    }
  }
   if (topic == 'colour'){
    insertMessage(topic, message)
  } if (topic == 'objectproximity'){
    insertMessage(topic, message)
  } if (topic == 'objectcoordinates'){
    insertMessage(topic, message)
  } 

});

client.on("connect",function(){	
console.log("connected  "+ client.connected);
})

//handle errors
client.on("error",function(error){
console.log("Can't connect" + error);
process.exit(1)});

//publish
function publish(message){
console.log("publishing topic: instruction", " message: ", message);

  if (client.connected == true){
    client.publish('instruction', message);
  }
}


// Create and Save a new Instruction
exports.create = (req, res) => {
  // Validate request
  if (!req.body.instruction) {
    res.status(400).send({ message: "Content can not be empty!" });
    return;
  }

  // Create an Instruction
  const instruction = new Instruction({
    instruction: req.body.instruction,
    amount: req.body.amount,
    status: req.body.status ? req.body.status : false
  });

  // Save Instruction in the database
  instruction
    .save(instruction)
    .then(data => {
      res.send(data);
    })
    .catch(err => {
      res.status(500).send({
        message:
          err.message || "Some error occurred while creating the Instruction."
      });
    });

    var message = req.body.instruction + " " + req.body.amount

    publish(message)
  
};

// Retrieve all Instructions from the database.
exports.findAll = (req, res) => {
  const instruction = req.query.instruction;
  var condition = instruction ? { instruction: { $regex: new RegExp(instruction), $options: "i" } } : {};

  Instruction.find(condition)
    .then(data => {
      res.send(data);
      //console.log(data)
    })
    .catch(err => {
      res.status(500).send({
        message:
          err.message || "Some error occurred while retrieving instructions."
      });
    });
};

// Find a single Instruction with an id
exports.findOne = (req, res) => {
  const id = req.params.id;

  Instruction.findById(id)
    .then(data => {
      if (!data)
        res.status(404).send({ message: "Not found Instruction with id " + id });
      else res.send(data);
    })
    .catch(err => {
      res
        .status(500)
        .send({ message: "Error retrieving Instruction with id=" + id });
    });
};

// Update a Instruction by the id in the request
exports.update = (req, res) => {
  if (!req.body) {
    return res.status(400).send({
      message: "Data to update can not be empty!"
    });
  }

  const id = req.params.id;

  Instruction.findByIdAndUpdate(id, req.body, { useFindAndModify: false })
    .then(data => {
      if (!data) {
        res.status(404).send({
          message: `Cannot update Instruction with id=${id}. Maybe Instruction was not found!`
        });
      } else res.send({ message: "Instruction was updated successfully." });
    })
    .catch(err => {
      res.status(500).send({
        message: "Error updating Instruction with id=" + id
      });
    });
};

// Delete a Instruction with the specified id in the request
exports.delete = (req, res) => {
  const id = req.params.id;

  Instruction.findByIdAndRemove(id, { useFindAndModify: false })
    .then(data => {
      if (!data) {
        res.status(404).send({
          message: `Cannot delete Instruction with id=${id}. Maybe Instruction was not found!`
        });
      } else {
        res.send({
          message: "Instruction was deleted successfully!"
        });
      }
    })
    .catch(err => {
      res.status(500).send({
        message: "Could not delete Instruction with id=" + id
      });
    });
};

// Delete all Instructions from the database.
exports.deleteAll = (req, res) => {
  Instruction.deleteMany({})
    .then(data => {
      res.send({
        message: `${data.deletedCount} Instructions were deleted successfully!`
      });
    })
    .catch(err => {
      res.status(500).send({
        message:
          err.message || "Some error occurred while removing all instructions."
      });
    });
};

// Find all completed Instructions
exports.findAllCompleted = (req, res) => {
  Instruction.find({ status: true })
    .then(data => {
      res.send(data);
    })
    .catch(err => {
      res.status(500).send({
        message:
          err.message || "Some error occurred while retrieving instructions."
      });
    });
};
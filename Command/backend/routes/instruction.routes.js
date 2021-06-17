module.exports = app => {
    const instructions = require("../controllers/instruction.controller.js");
  
    var router = require("express").Router();
  
    // Create a new Instruction
    router.post("/", instructions.create);
  
    // Retrieve all Instructions
    router.get("/", instructions.findAll);
  
    // Retrieve all completed Instructions
    router.get("/complete", instructions.findAllCompleted);
  
    // Retrieve a single Instruction with id
    router.get("/:id", instructions.findOne);
  
    // Update a Instruction with id
    router.put("/:id", instructions.update);
  
    // Delete a Instruction with id
    router.delete("/:id", instructions.delete);
  
    // Create a new Instruction
    router.delete("/", instructions.deleteAll);
  
    app.use("/api/instructions", router);
  };
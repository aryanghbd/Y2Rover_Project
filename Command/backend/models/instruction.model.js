module.exports = mongoose => {
  var schema = mongoose.Schema(
    {
      instruction: String,
      amount: String,
      status: Boolean
    },
    { timestamps: true }
  );

  schema.method("toJSON", function() {
    const { __v, _id, ...object } = this.toObject();
    object.id = _id;
    return object;
  });

  const Instruction = mongoose.model("instruction", schema);
  return Instruction;
};
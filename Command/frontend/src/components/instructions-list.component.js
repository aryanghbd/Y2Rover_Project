import React, { Component } from "react";
import InstructionDataService from "../services/instruction.service";
import { Link } from "react-router-dom";

export default class InstructionList extends Component {
  constructor(props) {
    super(props);
    this.onChangeSearchInstruction = this.onChangeSearchInstruction.bind(this);
    this.retrieveInstructions = this.retrieveInstructions.bind(this);
    this.refreshList = this.refreshList.bind(this);
    this.setActiveInstruction = this.setActiveInstruction.bind(this);
    this.removeAllInstructions = this.removeAllInstructions.bind(this);
    this.searchInstruction = this.searchInstruction.bind(this);

    this.state = {
      instructions: [],
      currentInstruction: null,
      currentIndex: -1,
      searchInstruction: ""
    };
  }

  componentDidMount() {
    this.retrieveInstructions();
  }

  onChangeSearchInstruction(e) {
    const searchInstruction = e.target.value;

    this.setState({
      searchInstruction: searchInstruction
    });
  }

  retrieveInstructions() {
    InstructionDataService.getAll()
      .then(response => {
        this.setState({
          instructions: response.data
        });
        console.log(response.data);
      })
      .catch(e => {
        console.log(e);
      });
  }

  refreshList() {
    this.retrieveInstructions();
    this.setState({
      currentInstruction: null,
      currentIndex: -1
    });
  }

  setActiveInstruction(instruction, index) {
    this.setState({
      currentInstruction: instruction,
      currentIndex: index
    });
  }

  removeAllInstructions() {
    InstructionDataService.deleteAll()
      .then(response => {
        console.log(response.data);
        this.refreshList();
      })
      .catch(e => {
        console.log(e);
      });
  }

  searchInstruction() {
    this.setState({
      currentInstruction: null,
      currentIndex: -1
    });

    InstructionDataService.findByInstruction(this.state.searchInstruction)
      .then(response => {
        this.setState({
          instructions: response.data
        });
        console.log(response.data);
      })
      .catch(e => {
        console.log(e);
      });
  }

  render() {
    const { searchInstruction, instructions, currentInstruction, currentIndex } = this.state;

    return (
      <div className="list row">
        <div className="col-md-8">
          <div className="input-group mb-3">
            <input
              type="text"
              className="form-control"
              placeholder="Search by instruction"
              value={searchInstruction}
              onChange={this.onChangeSearchInstruction}
            />
            <div className="input-group-append">
              <button
                className="btn btn-outline-secondary"
                type="button"
                onClick={this.searchInstruction}
              >
                Search
              </button>
            </div>
          </div>
        </div>
        <div className="col-md-6">
          <h4>Instructions List</h4>

          <ul className="list-group">
            {instructions &&
              instructions.map((instruction, index) => (
                <li
                  className={
                    "list-group-item " +
                    (index === currentIndex ? "active" : "")
                  }
                  onClick={() => this.setActiveInstruction(instruction, index)}
                  key={index}
                >
                  {instruction.instruction}
                </li>
              ))}
          </ul>

          <button
            className="m-3 btn btn-sm btn-danger"
            onClick={this.removeAllInstructions}
          >
            Remove All
          </button>
        </div>
        <div className="col-md-6">
          {currentInstruction ? (
            <div>
              <h4>Instruction</h4>
              <div>
                <label>
                  <strong>Instruction:</strong>
                </label>{" "}
                {currentInstruction.instruction}
              </div>
              <div>
                <label>
                  <strong>Amount:</strong>
                </label>{" "}
                {currentInstruction.amount}
              </div>
              <div>
                <label>
                  <strong>Status:</strong>
                </label>{" "}
                {currentInstruction.status ? "Complete" : "Pending"}
              </div>

              <Link
                to={"/instructions/" + currentInstruction.id}
                className="m-3 btn btn-sm btn-warning"
              >
                Edit
              </Link>
            </div>
          ) : (
            <div>
              <br />
              <p>Please click on an Instruction...</p>
            </div>
          )}
        </div>
      </div>
    );
  }
}
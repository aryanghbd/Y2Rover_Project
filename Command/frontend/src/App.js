import React, { Component } from "react";
import { Switch, Route, Link } from "react-router-dom";
import "bootstrap/dist/css/bootstrap.min.css";
import "./App.css";

import Home from "./components/home.component";
import AddInstruction from "./components/add-instruction.component";
import Instruction from "./components/instruction.component";
import InstructionsList from "./components/instructions-list.component";
import Map from "./components/map.component";


class App extends Component {
  render() {
    return (
      <div>
        <nav className="navbar navbar-expand navbar-dark bg-dark">
          <Link to={"/home"} className="navbar-brand">
            Mars Rover
          </Link>
          <div className="navbar-nav mr-auto">
            <li className="nav-item">
              <Link to={"/instructions"} className="nav-link">
                Instructions
              </Link>
            </li>
            <li className="nav-item">
              <Link to={"/add"} className="nav-link">
                Add
              </Link>
            </li>
            <li className="nav-item">
              <Link to={"/map"} className="nav-link">
                Map
              </Link>
            </li>
          </div>
        </nav>

        <div className="container mt-3">
          <Switch>
            <Route exact path={["/", "/home"]} component={Home} />
            <Route exact path={["/", "/instructions"]} component={InstructionsList} />
            <Route exact path="/add" component={AddInstruction} />
            <Route exact path="/map" component={Map} />
            <Route path="/instructions/:id" component={Instruction} />
          </Switch>
        </div>
      </div>
    );
  }
}

export default App;
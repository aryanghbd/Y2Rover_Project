import http from "../http-common";

class InstructionDataService {
  getAll() {
    return http.get("/instructions");
  }

  get(id) {
    return http.get(`/instructions/${id}`);
  }

  create(data) {
    return http.post("/instructions", data);
  }

  update(id, data) {
    return http.put(`/instructions/${id}`, data);
  }

  delete(id) {
    return http.delete(`/instructions/${id}`);
  }

  deleteAll() {
    return http.delete(`/instructions`);
  }

  findByInstruction(instruction) {
    return http.get(`/instructions?instruction=${instruction}`);
  }

}

export default new InstructionDataService();
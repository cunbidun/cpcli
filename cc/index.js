import express from "express";
import {
  copyFileSync,
  existsSync,
  mkdirSync,
  readFileSync,
  writeFileSync,
} from "fs";
import path from "path";
import { exit } from "process";

function print_usage() {
  console.log("The correct usage is: ");
  console.log("   npm start -- config=<path/to/config/file>");
}

if (process.argv.length !== 3) {
  console.log("Invalid command line args!!!");
  print_usage();
  exit(1);
}

let config_array = process.argv[2].split("=");
if (config_array[0] !== "config") {
  console.log("Invalid command line args!!!");
  print_usage();
  exit(1);
}

let project_config_path = config_array[1];
console.log(`Using config file ${project_config_path} (assuming it exists)`);

let project_config = JSON.parse(readFileSync(project_config_path));

console.log(`Using task dir: ${project_config.task_dir} (assuming it exists)`);


const app = express();
app.use(express.json());

const PORT = 8080;

function reformat(s) {
  return s.replace(/\//g, `-`).replace(/["!'#:]/g, ``);
}

app.post("/", async (req, res) => {
  console.log(req.body);
  const data = req.body;
  data.name = reformat(data.name);
  data.group = reformat(data.group);

  const root_dir = path.join(project_config.task_dir, data.name);
  const root_solution_path = path.join(path.resolve(root_dir), "solution.cpp");
  const root_config_path = path.join(path.resolve(root_dir), "config.json");
  const template_dir = project_config.template_dir;
  const template_solution_path = path.join(template_dir, "solution.template");
  const template_config_path = path.join(template_dir, "config.template");

  mkdirSync(root_dir, { recursive: true }, (err) => {
    if (err) {
      console.log("mkdir failed:", err);
      return;
    }
  });

  let cnt = 0;
  let tests = data.tests.map((e) => ({
    input: e.input,
    output: e.output,
    index: cnt++,
    active: true,
  }));

  let taskData = {
    tests: tests,
    name: data.name,
    group: data.group,
    timeLimit: data.timeLimit !== null ? data.timeLimit : 10000,
    url: data.url,
  };

  if (!existsSync(root_solution_path)) {
    copyFileSync(template_solution_path, root_solution_path);
  }

  if (!existsSync(root_config_path)) {
    let config = JSON.parse(readFileSync(template_config_path));
    let obj = { ...config, ...taskData };
    writeFileSync(root_config_path, JSON.stringify(obj, null, 2));
  }
  res.sendStatus(200);
});

app.listen(PORT, (err) => {
  if (err) {
    console.error(err);
    process.exit(1);
  }
  console.log(`Listening on PORT ${PORT}`);
});

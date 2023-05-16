import fs from 'fs';
import { spawn } from 'child_process';

const program = process.argv[2], test_name = process.argv[3];

const hotel = spawn(`./${program}/bin/hotel.exe`, [ process.argv[4] ]);
let hotelOutput = ""; hotel.stdout.on('data', data => hotelOutput += data.toString());
await new Promise(r => setTimeout(r, 1000));

const test = fs.readFileSync(`testing/${test_name}_test.txt`, "utf-8").replaceAll("\r", "").split('\n');
let visitorsOutput = new Array(test.length / 2).fill(""), waiters = [ ];
for (let i = 0; i < test.length; i += 2)
{
    const visitor = spawn(`./${program}/bin/visitor.exe`, [ "127.0.0.1", process.argv[4], test[i], test[i + 1] ]);
    visitor.stdout.on('data', data => visitorsOutput[i / 2] += data.toString());
    waiters.push(new Promise(resolve => visitor.on('exit', resolve)));
}

await Promise.all(waiters);
await new Promise(r => setTimeout(r, 1000));
hotel.kill('SIGINT');
await new Promise(resolve => hotel.on('exit', resolve));

fs.rmSync(`${program}/output/${test_name}`, { recursive: true, force: true });
fs.mkdirSync(`${program}/output/${test_name}`, { recursive: true });
fs.writeFileSync(`${program}/output/${test_name}/hotel.txt`, hotelOutput, "utf-8");
for (let i = 0; i < visitorsOutput.length; i++) fs.writeFileSync(`${program}/output/${test_name}/visitor-${i}.txt`, visitorsOutput[i], "utf-8");
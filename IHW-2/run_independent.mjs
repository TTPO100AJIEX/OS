import fs from 'fs';
const program = process.argv[2], test_name = process.argv[3];
const test = fs.readFileSync(`${test_name}_test.txt`, "utf-8").replaceAll("\r", "").split('\n');

import { spawn } from 'child_process';

const hotel = spawn(`./${program}/bin/hotel.exe`);
let hotelOutput = "";
hotel.stdout.on('data', data => hotelOutput += data.toString());
await new Promise(r => setTimeout(r, 1000));

let visitors = [ ];
for (let i = 0; i < test.length; i += 2)
{
    visitors.push("");
    const visitor = spawn(`./${program}/bin/visitor.exe`, [ test[i], test[i + 1] ]);
    visitor.stdout.on('data', data => visitors[i / 2] += data.toString());
}

if (test_name == "small") await new Promise(r => setTimeout(r, 10000));
if (test_name == "big") await new Promise(r => setTimeout(r, 100000));
hotel.kill('SIGINT');

await new Promise(r => setTimeout(r, 1000));
fs.rmSync(`${program}/output/${test_name}`, { recursive: true, force: true });
fs.mkdirSync(`${program}/output/${test_name}`, { recursive: true });
fs.writeFileSync(`${program}/output/${test_name}/hotel.txt`, hotelOutput, "utf-8");
for (let i = 0; i < visitors.length; i++) fs.writeFileSync(`${program}/output/${test_name}/visitor-${i}.txt`, visitors[i], "utf-8");
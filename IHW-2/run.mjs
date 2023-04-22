import fs from 'fs';
const program = Number(process.argv[2]), test_name = process.argv[3];

import { spawn } from 'child_process';
if (program >= 4 && program <= 6)
{
    const worker = spawn(`./${program}/bin/index.exe`, [ `${program}/output/${test_name}.txt` ]);
    worker.stdin.write(fs.readFileSync(`${test_name}_test.txt`, "utf-8") + '\n');
    
    if (test_name == "small") await new Promise(r => setTimeout(r, 10000));
    if (test_name == "big") await new Promise(r => setTimeout(r, 100000));
    worker.kill('SIGINT');
}
else
{
    const hotel = spawn(`./${program}/bin/hotel.exe`);
    let hotelOutput = ""; hotel.stdout.on('data', data => hotelOutput += data.toString());
    await new Promise(r => setTimeout(r, 1000));

    const test = fs.readFileSync(`${test_name}_test.txt`, "utf-8").replaceAll("\r", "").split('\n');
    let visitorsOutput = new Array(test.length / 2).fill("");
    for (let i = 0; i < test.length; i += 2)
    {
        const visitor = spawn(`./${program}/bin/visitor.exe`, [ test[i], test[i + 1] ]);
        visitor.stdout.on('data', data => visitorsOutput[i / 2] += data.toString());
    }

    if (test_name == "small") await new Promise(r => setTimeout(r, 10000));
    if (test_name == "big") await new Promise(r => setTimeout(r, 100000));
    hotel.kill('SIGINT');

    await new Promise(r => setTimeout(r, 1000));
    fs.rmSync(`${program}/output/${test_name}`, { recursive: true, force: true });
    fs.mkdirSync(`${program}/output/${test_name}`, { recursive: true });
    fs.writeFileSync(`${program}/output/${test_name}/hotel.txt`, hotelOutput, "utf-8");
    for (let i = 0; i < visitorsOutput.length; i++) fs.writeFileSync(`${program}/output/${test_name}/visitor-${i}.txt`, visitorsOutput[i], "utf-8");
}
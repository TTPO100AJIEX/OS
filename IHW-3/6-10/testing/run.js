import fs from 'fs';
import { spawn } from 'child_process';

const hotel = spawn(`./6-10/bin/hotel.exe`, [ process.argv[3] ]);
var hotelOutput = ""; hotel.stdout.on('data', data => hotelOutput += data.toString());
await new Promise(r => setTimeout(r, 1000));



var loggersOutput = [ ], loggersWaiters = [ ];
const interval = setInterval(() =>
{
    const logger = spawn(`./6-10/bin/logger.exe`, [ "127.0.0.1", process.argv[3] ]);
    const loggerIndex = loggersOutput.length; loggersOutput.push("");
    loggersWaiters.push(new Promise(resolve => logger.on('exit', resolve)));
    logger.stdout.on('data', data => loggersOutput[loggerIndex] += data.toString());
}, 250);


const test = fs.readFileSync(`6-10/testing/${process.argv[2]}_test.txt`, "utf-8").replaceAll("\r", "").split('\n');
var visitorsOutput = new Array(test.length / 2).fill(""), waiters = [ ];
for (let i = 0; i < test.length; i += 2)
{
    const visitor = spawn(`./6-10/bin/visitor.exe`, [ "127.0.0.1", process.argv[3], test[i], test[i + 1] ]);
    visitor.stdout.on('data', data => visitorsOutput[i / 2] += data.toString());
    waiters.push(new Promise(resolve => visitor.on('exit', resolve)));
    await new Promise(r => setTimeout(r, 75));
}

Promise.all(waiters).then(async (data) =>
{
    clearInterval(interval);

    console.log(data);
    await new Promise(r => setTimeout(r, 1000));
    hotel.kill('SIGINT');
    await new Promise(resolve => hotel.on('exit', resolve));

    console.log(await Promise.all(loggersWaiters));
    
    fs.rmSync(`6-10/output/${process.argv[2]}`, { recursive: true, force: true });
    fs.mkdirSync(`6-10/output/${process.argv[2]}/visitors`, { recursive: true });
    fs.mkdirSync(`6-10/output/${process.argv[2]}/loggers`, { recursive: true });
    fs.writeFileSync(`6-10/output/${process.argv[2]}/hotel.txt`, hotelOutput, "utf-8");
    for (let i = 0; i < visitorsOutput.length; i++) fs.writeFileSync(`6-10/output/${process.argv[2]}/visitors/visitor-${i}.txt`, visitorsOutput[i], "utf-8");
    for (let i = 0; i < loggersOutput.length; i++) fs.writeFileSync(`6-10/output/${process.argv[2]}/loggers/logger-${i}.txt`, loggersOutput[i], "utf-8");
});
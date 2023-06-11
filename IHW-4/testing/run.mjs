import fs from 'fs';
import { spawn } from 'child_process';

const hotel = spawn(`./bin/hotel.exe`, [ process.argv[3], process.argv[4], process.argv[5] ]);
var hotelOutput = ""; hotel.stdout.on('data', data => hotelOutput += data.toString());
await new Promise(r => setTimeout(r, 1000));



var loggersOutput = [ ], loggersWaiters = [ ];
function create_logger()
{
    const logger = spawn(`./bin/logger.exe`, [ process.argv[4], process.argv[5] ]);
    const loggerIndex = loggersOutput.length; loggersOutput.push("");
    loggersWaiters.push(new Promise(resolve => logger.on('exit', resolve)));
    logger.stdout.on('data', data => loggersOutput[loggerIndex] += data.toString());
}
const interval = setInterval(create_logger, 250);
create_logger();


const test = fs.readFileSync(`testing/${process.argv[2]}_test.txt`, "utf-8").replaceAll("\r", "").split('\n');
var visitorsOutput = new Array(test.length / 2).fill(""), waiters = [ ];
for (let i = 0; i < test.length; i += 2)
{
    const visitor = spawn(`./bin/visitor.exe`, [ "127.0.0.1", process.argv[3], test[i], test[i + 1] ]);
    visitor.stdout.on('data', data => visitorsOutput[i / 2] += data.toString());
    waiters.push(new Promise(resolve => visitor.on('exit', resolve)));
    await new Promise(r => setTimeout(r, 75));
}

console.log(await Promise.all(waiters));
clearInterval(interval);

await new Promise(r => setTimeout(r, 1000));
hotel.kill('SIGINT');
await new Promise(resolve => hotel.on('exit', resolve));

console.log(await Promise.all(loggersWaiters));

fs.rmSync(`output/${process.argv[2]}`, { recursive: true, force: true });
fs.mkdirSync(`output/${process.argv[2]}/visitors`, { recursive: true });
fs.mkdirSync(`output/${process.argv[2]}/loggers`, { recursive: true });
fs.writeFileSync(`output/${process.argv[2]}/hotel.txt`, hotelOutput, "utf-8");
for (let i = 0; i < visitorsOutput.length; i++) fs.writeFileSync(`output/${process.argv[2]}/visitors/visitor-${i}.txt`, visitorsOutput[i], "utf-8");
for (let i = 0; i < loggersOutput.length; i++) fs.writeFileSync(`output/${process.argv[2]}/loggers/logger-${i}.txt`, loggersOutput[i], "utf-8");

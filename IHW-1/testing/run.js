import fs from 'fs';
import { execFile, execFileSync } from 'child_process';
import { promisify } from 'util';

process.argv[2] = Number(process.argv[2]);

const tests = fs.readdirSync("testing/tests").sort((a, b) => Number(a.slice(5)) - Number(b.slice(5)));
for (const test of tests)
{
    try
    {
        if (process.argv[2] >= 8)
        {
            const io = promisify(execFile)(`./${process.argv[2]}/bin/io.exe`, [ "testing/tests/" + test + "/in.in", process.argv[2] + "/output/" + test + ".out" ]);
            await new Promise(r => setTimeout(r, 250));
            const solver = promisify(execFile)(`./${process.argv[2]}/bin/solver.exe`);
            await Promise.allSettled([ io, solver ]);
        }
        else
        {
            execFileSync(`./${process.argv[2]}/bin/index.exe`, [ "testing/tests/" + test + "/in.in", process.argv[2] + "/output/" + test + ".out" ]);
        }
    } catch(err) { console.log(`⚠️ RE: test ${test.slice(5)}`, err); }
    
    const correctAnswer = fs.readFileSync("testing/tests/" + test + "/out.out")
    const givenAnswer = fs.readFileSync(process.argv[2] + "/output/" + test + ".out");
    if (correctAnswer.equals(givenAnswer)) console.log(`✅ OK: test ${test.slice(5)}`);
    else console.log(`❌ WA: test ${test.slice(5)}`);
}
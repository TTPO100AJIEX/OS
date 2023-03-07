import fs from 'fs';
import { execFileSync } from 'child_process';

fs.readdirSync("testing/tests").sort((a, b) => Number(a.slice(5)) - Number(b.slice(5))).forEach(folder =>
{
    try { execFileSync(`./${process.argv[2]}/bin/index.exe`, [ "testing/tests/" + folder + "/in.in", process.argv[2] + "/output/" + folder + ".out" ]); }
    catch(err) { console.log(`⚠️ RE: test ${folder.slice(5)}`, err); }
    
    const correctAnswer = fs.readFileSync("testing/tests/" + folder + "/out.out")
    const givenAnswer = fs.readFileSync(process.argv[2] + "/output/" + folder + ".out");
    if (correctAnswer.equals(givenAnswer)) console.log(`✅ OK: test ${folder.slice(5)}`);
    else console.log(`❌ WA: test ${folder.slice(5)}`);
});
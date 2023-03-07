import fs from 'fs';
fs.rmSync(`testing/tests`, { recursive: true, force: true });

for (let test_size = 1; test_size < 1e8; test_size *= 2) saveTest(test_size);
for (let test_size = 5; test_size < 1e8; test_size *= 10) saveTest(test_size);
for (let test_size = 10; test_size < 1e8; test_size *= 10) saveTest(test_size);

function saveTest(test_size)
{
    const { test, answer } = generateTest(test_size);
    fs.mkdirSync(`testing/tests/test-${test_size}`, { recursive: true });
    fs.writeFileSync(`testing/tests/test-${test_size}/in.in`, test, "utf-8");
    fs.writeFileSync(`testing/tests/test-${test_size}/out.out`, answer, "utf-8");
}
function generateTest(length)
{
    let test = "";
    for (let i = 0; i < length; i++)
    {
        // There are 52 letters and 42 delimiters. So probability of getting a delimiter is 0.45
        // To increase the length of the words, two characters are generated and the one that is not a delimiter is taken, if such exists.
        const char1 = String.fromCharCode(Math.floor(Math.random() * (126 - 33 + 1) + 33)), char2 = String.fromCharCode(Math.floor(Math.random() * (126 - 33 + 1) + 33));
        if ((char1 >= 'a' && char1 <= 'z') || (char1 >= 'A' && char1 <= 'Z')) test += char1;
        else test += char2;
    }

    const answer = test.split(/[^a-zA-Z]/).filter(word => word[0] >= 'A' && word[0] <= 'Z').join(' ');

    return { test, answer };
}
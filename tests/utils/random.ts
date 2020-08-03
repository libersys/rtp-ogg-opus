/**
 * Returns a random number between min (inclusive) and max (exclusive)
 * @param {number} min: the minimum possible number.
 * @param {number} max: the maximum possible number.
 */
export const randomNumber = (min: number, max: number) => {
    // FROM https://stackoverflow.com/questions/1527803/generating-random-whole-numbers-in-javascript-in-a-specific-range
    return Math.random() * (max - min) + min;
};

/**
 * Returns a random integer between min (inclusive) and max (inclusive).
 * @param {number} min: the minimum possible number.
 * @param {number} max: the maximum possible number.
 */
export const randomInt = (min: number, max: number) => {
    // FROM https://stackoverflow.com/questions/1527803/generating-random-whole-numbers-in-javascript-in-a-specific-range
    min = Math.ceil(min);
    max = Math.floor(max);
    return Math.floor(Math.random() * (max - min + 1)) + min;
};

/**
 * Returns a random string.
 * @param {number} length: the length of the string.
 */
export const randomString = (length: number = 64) => {
    // FROM https://gist.github.com/6174/6062387
    let ret = '';
    for (let i = 0; i < length / 5; i++) {
        ret +=
            i % 2
                ? Math.random().toString(36).substring(2, 7)
                : Math.random().toString(36).substring(2, 7).toUpperCase();
    }
    return ret.substring(0, length);
};

/**
 * Returns a random email.
 */
export const randomEmail = () => {
    return `${randomString(16)}@${randomString(16)}.com`;
};

export default {
    randomNumber,
    randomInt,
    randomString,
    randomEmail,
};

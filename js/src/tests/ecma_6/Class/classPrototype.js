var test = `

// The prototype of a class is a non-writable, non-configurable, non-enumerable data property.
class a { constructor() { } }
var protoDesc = Object.getOwnPropertyDescriptor(a, "prototype");
assertEq(protoDesc.writable, false);
assertEq(protoDesc.configurable, false);
assertEq(protoDesc.enumerable, false);

var prototype = protoDesc.value;
assertEq(typeof prototype, "object");
assertEq(Object.getPrototypeOf(prototype), Object.prototype);
assertEq(Object.isExtensible(prototype), true);

var desiredPrototype = {};
Object.defineProperty(desiredPrototype, "constructor", { writable: true,
                                                         configurable: true,
                                                         enumerable: false,
                                                         value: a });
assertDeepEq(prototype, desiredPrototype);

try {
    eval(\`class a {
            constructor() { };
            static [\"prototype\"]() { };
          }\`);
} catch (e if e instanceof TypeError) {
    throw new Error("Congrats on making initprop respect non-writable " +
                    "non-configurable properties. Uncomment the test below " +
                    "for bonus points.");
/*
// As such, it should by a TypeError to try and overwrite "prototype" with a
// static member. The only way to try is with a computed property name; the rest
// are early errors.
assertThrowsInstanceOf(() => eval(\`
                                  class a {
                                    constructor() { };
                                    static ["prototype"]() { }
                                  }
                                  \`), TypeError);
assertThrowsInstanceOf(() => eval(\`
                                  class a {
                                    constructor() { };
                                    static get ["prototype"]() { }
                                  }
                                  \`), TypeError);
assertThrowsInstanceOf(() => eval(\`
                                  class a {
                                    constructor() { };
                                    static set ["prototype"](x) { }
                                  }
                                  \`), TypeError);
*/
}
`;

if (classesEnabled())
    eval(test);

if (typeof reportCompare === "function")
    reportCompare(0, 0, "OK");

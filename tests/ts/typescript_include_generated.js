"use strict";
var __defProp = Object.defineProperty;
var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
var __getOwnPropNames = Object.getOwnPropertyNames;
var __hasOwnProp = Object.prototype.hasOwnProperty;
var __export = (target, all) => {
  for (var name in all)
    __defProp(target, name, { get: all[name], enumerable: true });
};
var __copyProps = (to, from, except, desc) => {
  if (from && typeof from === "object" || typeof from === "function") {
    for (let key of __getOwnPropNames(from))
      if (!__hasOwnProp.call(to, key) && key !== except)
        __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
  }
  return to;
};
var __toCommonJS = (mod) => __copyProps(__defProp({}, "__esModule", { value: true }), mod);

// foobar.ts
var foobar_exports = {};
__export(foobar_exports, {
  Abc: () => Abc,
  class_: () => class_
});
module.exports = __toCommonJS(foobar_exports);

// foobar/abc.js
var Abc;
(function(Abc2) {
  Abc2[Abc2["a"] = 0] = "a";
})(Abc = Abc || (Abc = {}));

// foobar/class.js
var class_;
(function(class_2) {
  class_2[class_2["arguments_"] = 0] = "arguments_";
})(class_ = class_ || (class_ = {}));

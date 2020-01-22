require('./jsdeps/gl-matrix');
function __c(o) { return new o.__proto__.constructor(); }
function __set_arr(self, args, required, name) {
    const argArr = args ? Array.from(args) : [];
    if (argArr.length === 0) {
        // already initialized to 0, not sure about other formats
        if (self.__arr instanceof Float32Array) return self;
        
        for(let i = 0;i < required;i++) argArr.push(0);
    }
    
    const vals = [];
    argArr.forEach(a => {
        if (a === null || a === undefined) return;
        const isArr = a instanceof Array;
        if (isArr || a.__arr) {
            const arr = isArr ? a : a.__arr;
            arr.forEach(v => vals.push(v));
        } else vals.push(a);
    });
    
    if (vals.length != required) {
        engine.error(`Invalid parameters specified to ${name}.set(). Parameters can be arrays, vec**s, mat**s, or quat*s, as long as the number of values stored within them is equal to ${required}. ${vals.length} value${vals.length === 1 ? '' : 's'} were found.`);
    } else self.__arr.set(vals);
    return self;
}
function __new_arr(copyTypeFrom, count) { return new copyTypeFrom.__arr.__proto__.constructor(count); }

var DEG_TO_RAD = 0.01745329251;
var RAD_TO_DEG = 57.2957795131;

function UseRadians() {
    DEG_TO_RAD = 1.0;
    RAD_TO_DEG = 1.0;
}

function UseDegrees() {
    DEG_TO_RAD = 0.01745329251;
    RAD_TO_DEG = 57.2957795131;
}

class vec4 {
    constructor(arr, args) {
        this.__xyz = null;
        this.__yzw = null;
        this.__xy = null;
        this.__yz = null;
        this.__zw = null;
        if (args.__view) this.__arr = new arr.__proto__.constructor(arr.buffer, arr.byteOffset + (args.__view[0] * 4), args.__view[1]);
        else {
            this.__arr = arr; 
            __set_arr(this, args, 4, 'vec4*');
        }
    }
    
    clone() { const out = __c(this); out.__arr.set(this.__arr); return out; }
    set() { return __set_arr(this, arguments, 4, 'vec4*'); }
    
    add(o) { const out = __c(this); _glMatrix._vec4.add(out.__arr, this.__arr, o.__arr); return out; }
    addEq(o) { _glMatrix._vec4.add(this.__arr, this.__arr, o.__arr); return this; }
    sub(o) { const out = __c(this); _glMatrix._vec4.subtract(out.__arr, this.__arr, o.__arr); return out; }
    subEq(o) { _glMatrix._vec4.subtract(this.__arr, this.__arr, o.__arr); return this; }
    mul(o) { const out = __c(this); _glMatrix._vec4.multiply(out.__arr, this.__arr, o.__arr); return out; }
    mulEq(o) { _glMatrix._vec4.multiply(this.__arr, this.__arr, o.__arr); return this; }
    div(o) { const out = __c(this); _glMatrix._vec4.divide(out.__arr, this.__arr, o.__arr); return out; }
    divEq(o) { _glMatrix._vec4.divide(this.__arr, this.__arr, o.__arr); return this; }
    
    transformed(o) {
        const out = __c(this);
        if (o instanceof mat4) _glMatrix._vec4.transformMat4(out.__arr, this.__arr, o.__arr);
        else if (o instanceof quat) _glMatrix._vec4.transformQuat(out.__arr, this.__arr, o.__arr);
        return out;
    }
    transform(o) {
        if (o instanceof mat4) _glMatrix._vec4.transformMat4(this.__arr, this.__arr, o.__arr);
        else if (o instanceof quat) _glMatrix._vec4.transformQuat(this.__arr, this.__arr, o.__arr);
        return this;
    }
    
    ceil() { const out = __c(this); _glMatrix._vec4.ceil(out.__arr, this.__arr); return out; }
    ceilEq() { _glMatrix._vec4.ceil(this.__arr, this.__arr); return this; }
    floor() { const out = __c(this); _glMatrix._vec4.floor(out.__arr, this.__arr); return out; }
    floorEq() { _glMatrix._vec4.floor(this.__arr, this.__arr); return this; }
    zero() { _glMatrix._vec4.zero(this.__arr); return this; }
    
    distance(o) { return _glMatrix._vec4.distance(this.__arr, o.__arr); }
    squaredDistance(o) { return _glMatrix._vec4.squaredDistance(this.__arr, o.__arr); }
    dot(o) { return _glMatrix._vec4.dot(this.__arr, o.__arr); }
    equals(o) { return _glMatrix._vec4.equals(this.__arr, o.__arr); }
    exactEquals(o) { return _glMatrix._vec4.exactEquals(this.__arr, o.__arr); }
    inverse() { const out = __c(this); _glMatrix._vec4.inverse(out.__arr, this.__arr); return out; }
    invert() { _glMatrix._vec4.inverse(this.__arr, this.__arr); return this; }
    negated() { const out = __c(this); _glMatrix._vec4.negate(out.__arr, this.__arr); return out; }
    negate() { _glMatrix._vec4.negate(this.__arr, this.__arr); return this; }
    normalized() { const out = __c(this); _glMatrix._vec4.normalize(out.__arr, this.__arr); return out; }
    normalize() { _glMatrix._vec4.normalize(this.__arr, this.__arr); return this; }
    rounded() { const out = __c(this); _glMatrix._vec4.round(out.__arr, this.__arr); return out; }
    round() { _glMatrix._vec4.round(this.__arr, this.__arr); return this; }
    scaled(fac) { const out = __c(this); _glMatrix._vec4.scale(out.__arr, this.__arr, fac); return out; }
    scale(fac) { _glMatrix._vec4.scale(this.__arr, this.__arr, fac); return this; }
    toString(digits = 2) { return `${this.__arr[0].toFixed(digits)}, ${this.__arr[1].toFixed(digits)}, ${this.__arr[2].toFixed(digits)}, ${this.__arr[3].toFixed(digits)}`; }
    
    static cross(a, b, c) { const out = __c(a); _glMatrix._vec4.cross(out.__arr, a.__arr, b.__arr, c.__arr); return out; }
    static lerp(a, b, t) { const out = __c(a); _glMatrix._vec4.lerp(out.__arr, a.__arr, b.__arr, t); return out; }
    static min(a, b) { const out = __c(a); _glMatrix._vec4.min(out.__arr, a.__arr, b.__arr); return out; }
    static max(a, b) { const out = __c(a); _glMatrix._vec4.max(out.__arr, a.__arr, b.__arr); return out; }
    
    get squaredLength() { return _glMatrix._vec4.squaredLength(this.__arr); }
    get length() { return _glMatrix._vec4.length(this.__arr); }
    set length(len) { this.normalize(); this.scale(len); }
    get x() { return this.__arr[0]; }
    set x(x) { return this.__arr[0] = x; }
    get y() { return this.__arr[1]; }
    set y(y) { return this.__arr[1] = y; }
    get z() { return this.__arr[2]; }
    set z(z) { return this.__arr[2] = z; }
    get w() { return this.__arr[3]; }
    set w(w) { return this.__arr[3] = w; }
    get xyz() {
        if (!this.__xyz) this.__xyz = new vec3(this.__arr, { __view: [0, 3] });
        return this.__xyz;
    }
    set xyz(v3) {
        this.__arr[0] = v3.__arr[0];
        this.__arr[1] = v3.__arr[1];
        this.__arr[2] = v3.__arr[2];
    }
    get yzw() {
        if (!this.__yzw) this.__yzw = new vec3(this.__arr, { __view: [1, 3] });
        return this.__yzw;
    }
    set yzw(v3) {
        this.__arr[1] = v3.__arr[0];
        this.__arr[2] = v3.__arr[1];
        this.__arr[3] = v3.__arr[2];
    }
    get xy() {
        if (!this.__xy) this.__xy = new vec2(this.__arr, { __view: [0, 2] });
        return this.__xy;
    }
    set xy(v2) {
        this.__arr[0] = v2.__arr[0];
        this.__arr[1] = v2.__arr[1];
    }
    get yz() {
        if (!this.__yz) this.__yz = new vec2(this.__arr, { __view: [1, 2] });
        return this.__yz;
    }
    set yz(v2) {
        this.__arr[1] = v2.__arr[0];
        this.__arr[2] = v2.__arr[1];
    }
    get zw() {
        if (!this.__zw) this.__zw = new vec2(this.__arr, { __view: [2, 2] });
        return this.__zw;
    }
    set zw(v2) {
        this.__arr[2] = v2.__arr[0];
        this.__arr[3] = v2.__arr[1];
    }
};

class vec3 {
    constructor(arr, args) {
        this.__xy = null;
        this.__yz = null;
        if (args.__view) this.__arr = new arr.__proto__.constructor(arr.buffer, arr.byteOffset + (args.__view[0] * 4), args.__view[1]);
        else {
            this.__arr = arr; 
            __set_arr(this, args, 3, 'vec3*');
        }
    }
    
    clone() { const out = __c(this); out.__arr.set(this.__arr); return out; }
    set() { return __set_arr(this, arguments, 3, 'vec3*'); }
    
    add(o) { const out = __c(this); _glMatrix._vec3.add(out.__arr, this.__arr, o.__arr); return out; }
    addEq(o) { _glMatrix._vec3.add(this.__arr, this.__arr, o.__arr); return this; }
    sub(o) { const out = __c(this); _glMatrix._vec3.subtract(out.__arr, this.__arr, o.__arr); return out; }
    subEq(o) { _glMatrix._vec3.subtract(this.__arr, this.__arr, o.__arr); return this; }
    mul(o) { const out = __c(this); _glMatrix._vec3.multiply(out.__arr, this.__arr, o.__arr); return out; }
    mulEq(o) { _glMatrix._vec3.multiply(this.__arr, this.__arr, o.__arr); return this; }
    div(o) { const out = __c(this); _glMatrix._vec3.divide(out.__arr, this.__arr, o.__arr); return out; }
    divEq(o) { _glMatrix._vec3.divide(this.__arr, this.__arr, o.__arr); return this; }
    
    transformed(o) {
        const out = __c(this);
        if (o instanceof mat4) _glMatrix._vec3.transformMat4(out.__arr, this.__arr, o.__arr);
        else if (o instanceof quat) _glMatrix._vec3.transformQuat(out.__arr, this.__arr, o.__arr);
        else if (o instanceof mat3) _glMatrix._vec3.transformMat3(out.__arr, this.__arr, o.__arr);
        return out;
    }
    transform(o) {
        if (o instanceof mat4) _glMatrix._vec3.transformMat4(this.__arr, this.__arr, o.__arr);
        else if (o instanceof quat) _glMatrix._vec3.transformQuat(this.__arr, this.__arr, o.__arr);
        else if (o instanceof mat3) _glMatrix._vec3.transformMat3(this.__arr, this.__arr, o.__arr);
        return this;
    }
    
    ceil() { const out = __c(this); _glMatrix._vec3.ceil(out.__arr, this.__arr); return out; }
    ceilEq() { _glMatrix._vec3.ceil(this.__arr, this.__arr); return this; }
    floor() { const out = __c(this); _glMatrix._vec3.floor(out.__arr, this.__arr); return out; }
    floorEq() { _glMatrix._vec3.floor(this.__arr, this.__arr); return this; }
    zero() { _glMatrix._vec3.zero(this.__arr); return this; }
    
    distance(o) { return _glMatrix._vec3.distance(this.__arr, o.__arr); }
    squaredDistance(o) { return _glMatrix._vec3.squaredDistance(this.__arr, o.__arr); }
    dot(o) { return _glMatrix._vec3.dot(this.__arr, o.__arr); }
    equals(o) { return _glMatrix._vec3.equals(this.__arr, o.__arr); }
    exactEquals(o) { return _glMatrix._vec3.exactEquals(this.__arr, o.__arr); }
    inverse() { const out = __c(this); _glMatrix._vec3.inverse(out.__arr, this.__arr); return out; }
    invert() { _glMatrix._vec3.inverse(this.__arr, this.__arr); return this; }
    negated() { const out = __c(this); _glMatrix._vec3.negate(out.__arr, this.__arr); return out; }
    negate() { _glMatrix._vec3.negate(this.__arr, this.__arr); return this; }
    normalized() { const out = __c(this); _glMatrix._vec3.normalize(out.__arr, this.__arr); return out; }
    normalize() { _glMatrix._vec3.normalize(this.__arr, this.__arr); return this; }
    rounded() { const out = __c(this); _glMatrix._vec3.round(out.__arr, this.__arr); return out; }
    round() { _glMatrix._vec3.round(this.__arr, this.__arr); return this; }
    scaled(fac) { const out = __c(this); _glMatrix._vec3.scale(out.__arr, this.__arr, fac); return out; }
    scale(fac) { _glMatrix._vec3.scale(this.__arr, this.__arr, fac); return this; }
    toString(digits = 2) { return `${this.__arr[0].toFixed(digits)}, ${this.__arr[1].toFixed(digits)}, ${this.__arr[2].toFixed(digits)}`; }
    
    static cross(a, b) { const out = __c(a); _glMatrix._vec3.cross(out.__arr, a.__arr, b.__arr); return out; }
    static angle(a, b) { _glMatrix._vec3.angle(out.__arr, a.__arr, b.__arr); }
    static lerp(a, b, t) { const out = __c(a); _glMatrix._vec3.lerp(out.__arr, a.__arr, b.__arr, t); return out; }
    static min(a, b) { const out = __c(a); _glMatrix._vec3.min(out.__arr, a.__arr, b.__arr); return out; }
    static max(a, b) { const out = __c(a); _glMatrix._vec3.max(out.__arr, a.__arr, b.__arr); return out; }
    static rotateX(point, origin, angle) { const out = __c(point); _glMatrix._vec3.rotateX(out.__arr, point.__arr, origin.__arr, angle * DEG_TO_RAD); return out; }
    static rotateY(point, origin, angle) { const out = __c(point); _glMatrix._vec3.rotateY(out.__arr, point.__arr, origin.__arr, angle * DEG_TO_RAD); return out; }
    static rotateZ(point, origin, angle) { const out = __c(point); _glMatrix._vec3.rotateZ(out.__arr, point.__arr, origin.__arr, angle * DEG_TO_RAD); return out; }
    
    get squaredLength() { return _glMatrix._vec3.squaredLength(this.__arr); }
    get length() { return _glMatrix._vec3.length(this.__arr); }
    set length(len) { this.normalize(); this.scale(len); }
    get x() { return this.__arr[0]; }
    set x(x) { return this.__arr[0] = x; }
    get y() { return this.__arr[1]; }
    set y(y) { return this.__arr[1] = y; }
    get z() { return this.__arr[2]; }
    set z(z) { return this.__arr[2] = z; }
    get xy() {
        if (!this.__xy) this.__xy = new vec2(this.__arr, { __view: [0, 2] });
        return this.__xy;
    }
    set xy(v2) {
        this.__arr[0] = v2.__arr[0];
        this.__arr[1] = v2.__arr[1];
    }
    get yz() {
        if (!this.__yz) this.__yz = new vec2(this.__arr, { __view: [1, 2] });
        return this.__yz;
    }
    set yz(v2) {
        this.__arr[1] = v2.__arr[0];
        this.__arr[2] = v2.__arr[1];
    }
};

class vec2 {
    constructor(arr, args) {
        if (args.__view) this.__arr = new arr.__proto__.constructor(arr.buffer, arr.byteOffset + (args.__view[0] * 4), args.__view[1]);
        else {
            this.__arr = arr; 
            __set_arr(this, args, 2, 'vec2*');
        }
    }
    
    clone() { const out = __c(this); out.__arr.set(this.__arr); return out; }
    set(x, y) { return __set_arr(this, [x, y], 2, 'vec2*'); }
    
    add(o) { const out = __c(this); _glMatrix._vec2.add(out.__arr, this.__arr, o.__arr); return out; }
    addEq(o) { _glMatrix._vec2.add(this.__arr, this.__arr, o.__arr); return this; }
    sub(o) { const out = __c(this); _glMatrix._vec2.subtract(out.__arr, this.__arr, o.__arr); return out; }
    subEq(o) { _glMatrix._vec2.subtract(this.__arr, this.__arr, o.__arr); return this; }
    mul(o) { const out = __c(this); _glMatrix._vec2.multiply(out.__arr, this.__arr, o.__arr); return out; }
    mulEq(o) { _glMatrix._vec2.multiply(this.__arr, this.__arr, o.__arr); return this; }
    div(o) { const out = __c(this); _glMatrix._vec2.divide(out.__arr, this.__arr, o.__arr); return out; }
    divEq(o) { _glMatrix._vec2.divide(this.__arr, this.__arr, o.__arr); return this; }
    
    transformed(o) {
        const out = __c(this);
        if (o instanceof mat4) _glMatrix._vec2.transformMat4(out.__arr, this.__arr, o.__arr);
        else if (o instanceof mat3) _glMatrix._vec2.transformMat3(out.__arr, this.__arr, o.__arr);
        else if (o instanceof mat2) _glMatrix._vec2.transformMat2(out.__arr, this.__arr, o.__arr);
        return out;
    }
    transform(o) {
        if (o instanceof mat4) _glMatrix._vec2.transformMat4(this.__arr, this.__arr, o.__arr);
        else if (o instanceof mat3) _glMatrix._vec2.transformMat3(this.__arr, this.__arr, o.__arr);
        else if (o instanceof mat2) _glMatrix._vec2.transformMat2(this.__arr, this.__arr, o.__arr);
        return this;
    }
    
    ceil() { const out = __c(this); _glMatrix._vec2.ceil(out.__arr, this.__arr); return out; }
    ceilEq() { _glMatrix._vec2.ceil(this.__arr, this.__arr); return this; }
    floor() { const out = __c(this); _glMatrix._vec2.floor(out.__arr, this.__arr); return out; }
    floorEq() { _glMatrix._vec2.floor(this.__arr, this.__arr); return this; }
    zero() { _glMatrix._vec2.zero(this.__arr); return this; }
    
    distance(o) { return _glMatrix._vec2.distance(this.__arr, o.__arr); }
    squaredDistance(o) { return _glMatrix._vec2.squaredDistance(this.__arr, o.__arr); }
    dot(o) { return _glMatrix._vec2.dot(this.__arr, o.__arr); }
    equals(o) { return _glMatrix._vec2.equals(this.__arr, o.__arr); }
    exactEquals(o) { return _glMatrix._vec2.exactEquals(this.__arr, o.__arr); }
    inverse() { const out = __c(this); _glMatrix._vec2.inverse(out.__arr, this.__arr); return out; }
    invert() { _glMatrix._vec2.inverse(this.__arr, this.__arr); return this; }
    negated() { const out = __c(this); _glMatrix._vec2.negate(out.__arr, this.__arr); return out; }
    negate() { _glMatrix._vec2.negate(this.__arr, this.__arr); return this; }
    normalized() { const out = __c(this); _glMatrix._vec2.normalize(out.__arr, this.__arr); return out; }
    normalize() { _glMatrix._vec2.normalize(this.__arr, this.__arr); return this; }
    rounded() { const out = __c(this); _glMatrix._vec2.round(out.__arr, this.__arr); return out; }
    round() { _glMatrix._vec2.round(this.__arr, this.__arr); return this; }
    scaled(fac) { const out = __c(this); _glMatrix._vec2.scale(out.__arr, this.__arr, fac); return out; }
    scale(fac) { _glMatrix._vec2.scale(this.__arr, this.__arr, fac); return this; }
    toString(digits = 2) { return `${this.__arr[0].toFixed(digits)}, ${this.__arr[1].toFixed(digits)}`; }
    
    static cross(a, b) { const out = __c(a); _glMatrix._vec2.cross(out.__arr, a.__arr, b.__arr); return out; }
    static angle(a, b) { _glMatrix._vec2.angle(out.__arr, a.__arr, b.__arr); }
    static lerp(a, b, t) { const out = __c(a); _glMatrix._vec2.lerp(out.__arr, a.__arr, b.__arr, t); return out; }
    static min(a, b) { const out = __c(a); _glMatrix._vec2.min(out.__arr, a.__arr, b.__arr); return out; }
    static max(a, b) { const out = __c(a); _glMatrix._vec2.max(out.__arr, a.__arr, b.__arr); return out; }
    static rotate(point, origin, angle) { const out = __c(point); _glMatrix._vec2.rotate(out.__arr, point.__arr, origin.__arr, angle * DEG_TO_RAD); return out; }
    
    get squaredLength() { return _glMatrix._vec2.squaredLength(this.__arr); }
    get length() { return _glMatrix._vec2.length(this.__arr); }
    set length(len) { this.normalize(); this.scale(len); }
    get x() { return this.__arr[0]; }
    set x(x) { return this.__arr[0] = x; }
    get y() { return this.__arr[1]; }
    set y(y) { return this.__arr[1] = y; }
};

class quat {
    constructor() {
        this.__arr = new Float32Array(4);
        if (Object.keys(arguments).length > 0) __set_arr(this, arguments, 4, 'quat');
        else this.__arr[3] = 1.0;
    }
    
    set() {
        __set_arr(this, arguments, 4, 'quat');
    }
    
    setIdentity() {
        this.__arr[0] = 0;
        this.__arr[1] = 0;
        this.__arr[2] = 0;
        this.__arr[3] = 1;
    }
    
    equals(o) { return _glMatrix._quat.equals(this.__arr, o.__arr); }
    equalsExact(o) { return _glMatrix._quat.equalsExact(this.__arr, o.__arr); }
    setAxes(forward, right, up) { _glMatrix._quat.setAxes(this.__arr, forward.__arr, right.__arr, up.__arr); return this; }
    setAxisAngle(axis, angle) { _glMatrix._quat.setAxisAngle(this.__arr, axis.__arr, angle * DEG_TO_RAD); return this; }
    
    add(o) { const out = new quat(); _glMatrix._quat.add(out.__arr, this.__arr, o.__arr); return out; }
    addEq(o) { _glMatrix._quat.add(this.__arr, this.__arr, o.__arr); return this; }
    
    mul(o) { const out = new quat(); _glMatrix._quat.multiply(out.__arr, this.__arr, o.__arr); return out; }
    mulEq(o) { _glMatrix._quat.multiply(this.__arr, this.__arr, o.__arr); return this; }
    
    scaled(fac) { const out = __c(this); _glMatrix._quat.scale(out.__arr, this.__arr, fac); return out; }
    scale(fac) { _glMatrix._quat.scale(this.__arr, this.__arr, fac); return this; }
    
    rotatedX(angle) { const out = new quat(); _glMatrix._quat.rotateX(out.__arr, this.__arr, angle * DEG_TO_RAD); return out; }
    rotateX(angle) { _glMatrix._quat.rotateX(this.__arr, this.__arr, angle * DEG_TO_RAD); return this; }
    rotatedY(angle) { const out = new quat(); _glMatrix._quat.rotateY(out.__arr, this.__arr, angle * DEG_TO_RAD); return out; }
    rotateY(angle) { _glMatrix._quat.rotateY(this.__arr, this.__arr, angle * DEG_TO_RAD); return this; }
    rotatedZ(angle) { const out = new quat(); _glMatrix._quat.rotateZ(out.__arr, this.__arr, angle * DEG_TO_RAD); return out; }
    rotateZ(angle) { _glMatrix._quat.rotateZ(this.__arr, this.__arr, angle * DEG_TO_RAD); return this; }
    
    calculateW() { return Math.sqrt(Math.abs(1.0 - this.__arr[0] * this.__arr[0] - this.__arr[1] * this.__arr[1] - this.__arr[2] * this.__arr[2])); }
    dot(o) { return _glMatrix._quat.dot(this.__arr, o.__arr); }
    exp() { const out = new quat(); _glMatrix._quat.exp(out.__arr, this.__arr); return out; }
    ln() { const out = new quat(); _glMatrix._quat.ln(out.__arr, this.__arr); return out; }
    pow(fac) { const out = new quat(); _glMatrix._quat.pow(out.__arr, this.__arr, fac); return out; }
    
    inverse() { const out = new quat(); _glMatrix._quat.invert(out.__arr, this.__arr); return out }
    invert() { _glMatrix._quat.invert(this.__arr, this.__arr); return this }
    normalized() { const out = new quat(); _glMatrix._quat.normalize(out.__arr, this.__arr); return out; }
    normalize() { _glMatrix._quat.normalize(this.__arr, this.__arr); return this; }
    conjugate() { const out = new quat(); _glMatrix._quat.conjugate(out.__arr, this.__arr); return out; }
    toAxisAngle() { const out = { axis: new vec3f(), angle: 0.0 }; out.angle = _glMatrix._quat.getAxisAngle(out.axis.__arr, this.__arr) * RAD_TO_DEG; return out; }
    
    static lerp(a, b, factor) { const out = new quat(); _glMatrix._quat.lerp(out.__arr, a.__arr, b.__arr, factor); return out; }
    static slerp(a, b, factor) { const out = new quat(); _glMatrix._quat.slerp(out.__arr, a.__arr, b.__arr, factor); return out; }
    static sqlerp(a, b, c, d, factor) { const out = new quat(); _glMatrix._quat.slerp(out.__arr, a.__arr, b.__arr, c.__arr, d.__arr, factor); return out; }
    static rotationTo(a, b) { const out = new quat(); _glMatrix._quat.rotationTo(out.__arr, a.__arr, b.__arr); return out; }
    static fromEuler(x, y, z) { const out = new quat(); _glMatrix._quat.fromEuler(out.__arr, x * DEG_TO_RAD, y * DEG_TO_RAD, z * DEG_TO_RAD); return out; }
    static fromMat3(mat) { const out = new quat(); _glMatrix._quat.fromMat3(out.__arr, mat.__arr); return out; }
    static fromAxisAngle(axis, angle) { const out = new quat(); _glMatrix._quat.setAxisAngle(out.__arr, axis.__arr, angle * DEG_TO_RAD); return out; }
    static angle(a, b) { return _glMatrix._quat.angle(a.__arr, b.__arr) * RAD_TO_DEG; }
    static random() { const out = new quat(); _glMatrix._quat.random(out.__arr); return out; }
    
    toString(digits = 2) { const { axis, angle } = this.toAxisAngle(); return `axis: ${axis.x.toFixed(digits)}, ${axis.y.toFixed(digits)}, ${axis.z.toFixed(digits)} angle: ${angle.toFixed(digits)}`; }
    
    get length() { return _glMatrix._quat.length(this.__arr); }
    get squaredLength() { return _glMatrix._quat.squaredLength(this.__arr); }
};

class mat4 {
    constructor(arr, args) {
        this.__arr = arr;
        this.__x = null;
        this.__y = null;
        this.__z = null;
        this.__w = null;
        __set_arr(this, args, 16, 'mat4*');
    }
    
    clone() { const out = __c(this); out.__arr.set(this.__arr); return out; }
    set() { return __set_arr(this, arguments, 16, 'mat4*'); }
    identity() { _glMatrix._mat4.identity(this.__arr); return this; }
    
    add(o) { const out = __c(this); _glMatrix._mat4.add(out.__arr, this.__arr, o.__arr); return out; }
    addEq(o) { _glMatrix._mat4.add(this.__arr, this.__arr, o.__arr); return this; }
    sub(o) { const out = __c(this); _glMatrix._mat4.subtract(out.__arr, this.__arr, o.__arr); return out; }
    subEq(o) { _glMatrix._mat4.subtract(this.__arr, this.__arr, o.__arr); return this; }
    mul(o) { const out = __c(this); _glMatrix._mat4.multiply(out.__arr, this.__arr, o.__arr); return out; }
    mulEq(o) { _glMatrix._mat4.multiply(this.__arr, this.__arr, o.__arr); return this; }
    mulScalar(o) { const out = __c(this); _glMatrix._mat4.multiplyScalar(out.__arr, this.__arr, o); return out; }
    mulScalarEq(o) { _glMatrix._mat4.multiplyScalar(this.__arr, this.__arr, o); return this; }
    equals(o) { return _glMatrix._mat4.equals(this.__arr, o.__arr); }
    equalsExact(o) { return _glMatrix._mat4.equalsExact(this.__arr, o.__arr); }
    inverse() { const out = __c(this); _glMatrix._mat4.invert(out.__arr, this.__arr); return out; }
    invert() { _glMatrix._mat4.invert(this.__arr, this.__arr); return this; }
    
    rotated(axis, angle) { const out = __c(this); _glMatrix._mat4.rotate(out.__arr, this.__arr, angle * DEG_TO_RAD, axis.__arr); return out; }
    rotate(axis, angle) { _glMatrix._mat4.rotate(this.__arr, this.__arr, angle * DEG_TO_RAD, axis.__arr); return this; }
    rotatedX(axis, angle) { const out = __c(this); _glMatrix._mat4.rotateX(out.__arr, this.__arr, angle * DEG_TO_RAD); return out; }
    rotateX(axis, angle) { _glMatrix._mat4.rotateX(this.__arr, this.__arr, angle * DEG_TO_RAD); return this; }
    rotatedY(axis, angle) { const out = __c(this); _glMatrix._mat4.rotateY(out.__arr, this.__arr, angle * DEG_TO_RAD); return out; }
    rotateY(axis, angle) { _glMatrix._mat4.rotateY(this.__arr, this.__arr, angle * DEG_TO_RAD); return this; }
    rotatedZ(axis, angle) { const out = __c(this); _glMatrix._mat4.rotateZ(out.__arr, this.__arr, angle * DEG_TO_RAD); return out; }
    rotateZ(axis, angle) { _glMatrix._mat4.rotateZ(this.__arr, this.__arr, angle * DEG_TO_RAD); return this; }
    scaled(s) { const out = __c(this); _glMatrix._mat4.scale(out.__arr, this.__arr, s.__arr); return out; }
    scale(s) { _glMatrix._mat4.scale(this.__arr, this.__arr, s.__arr); return this; }
    translated(t) { const out = __c(this); _glMatrix._mat4.translate(out.__arr, this.__arr, t.__arr); return out; }
    translate(t) { _glMatrix._mat4.translate(this.__arr, this.__arr, t.__arr); return this; }
    transposed() { const out = __c(this); _glMatrix._mat4.transpose(out.__arr, this.__arr); return out; }
    transpose() { _glMatrix._mat4.transpose(this.__arr, this.__arr); return this; }
    
    adjoint() { const out = __c(this); _glMatrix._mat4.adjoint(out.__arr, this.__arr); return out; }
    determinant() { return _glMatrix._mat4.determinant(this.__arr); }
    frob() { return _glMatrix._mat4.frob(this.__arr); }
    getRotation() { const out = new quat(); _glMatrix._mat4.getRotation(out.__arr, this.__arr); return out; }
    getScale() { const out = new vec3(__new_arr(this, 3)); _glMatrix._mat4.getScaling(out.__arr, this.__arr); return out; }
    getTranslation() { const out = new vec3(__new_arr(this, 3)); _glMatrix._mat4.getTranslation(out.__arr, this.__arr); return out; }
    getNormalMatrix() { const out = new mat3(__new_arr(this, 9)); _glMatrix._mat3.normalFromMat4(out.__arr, this.__arr); return out; }
    
    toString(digits = 2) {
        return `${this.__arr[ 0].toFixed(digits)}, ${this.__arr[ 1].toFixed(digits)}, ${this.__arr[ 2].toFixed(digits)}, ${this.__arr[ 3].toFixed(digits)}\n` +
               `${this.__arr[ 4].toFixed(digits)}, ${this.__arr[ 5].toFixed(digits)}, ${this.__arr[ 6].toFixed(digits)}, ${this.__arr[ 7].toFixed(digits)}\n` +
               `${this.__arr[ 8].toFixed(digits)}, ${this.__arr[ 9].toFixed(digits)}, ${this.__arr[10].toFixed(digits)}, ${this.__arr[11].toFixed(digits)}\n` +
               `${this.__arr[12].toFixed(digits)}, ${this.__arr[13].toFixed(digits)}, ${this.__arr[14].toFixed(digits)}, ${this.__arr[15].toFixed(digits)}`;
    }
    
    get x() {
        if (!this.__x) this.__x = new vec4(this.__arr, { __view: [0, 4]});
        return this.__x;
    }
    set x(v4) {
        this.__arr[0] = v4.__arr[0];
        this.__arr[1] = v4.__arr[1];
        this.__arr[2] = v4.__arr[2];
        this.__arr[3] = v4.__arr[3];
    }
    get y() {
        if (!this.__y) this.__y = new vec4(this.__arr, { __view: [4, 4]});
        return this.__y;
    }
    set y(v4) {
        this.__arr[4] = v4.__arr[0];
        this.__arr[5] = v4.__arr[1];
        this.__arr[6] = v4.__arr[2];
        this.__arr[7] = v4.__arr[3];
    }
    get z() {
        if (!this.__z) this.__z = new vec4(this.__arr, { __view: [8, 4]});
        return this.__z;
    }
    set z(v4) {
        this.__arr[8] = v4.__arr[0];
        this.__arr[9] = v4.__arr[1];
        this.__arr[10] = v4.__arr[2];
        this.__arr[11] = v4.__arr[3];
    }
    get w() {
        if (!this.__w) this.__w = new vec4(this.__arr, { __view: [12, 4]});
        return this.__w;
    }
    set w(v4) {
        this.__arr[12] = v4.__arr[0];
        this.__arr[13] = v4.__arr[1];
        this.__arr[14] = v4.__arr[2];
        this.__arr[15] = v4.__arr[3];
    }
};

class mat3 {
    constructor(arr, args) {
        this.__arr = arr;
        this.__x = null;
        this.__y = null;
        this.__z = null;
        __set_arr(this, args, 9, 'mat3*');
    }
    
    clone() { const out = __c(this); out.__arr.set(this.__arr); return out; }
    set() { return __set_arr(this, arguments, 9, 'mat3*'); }
    identity() { _glMatrix._mat3.identity(this.__arr); return this; }
    
    add(o) { const out = __c(this); _glMatrix._mat3.add(out.__arr, this.__arr, o.__arr); return out; }
    addEq(o) { _glMatrix._mat3.add(this.__arr, this.__arr, o.__arr); return this; }
    sub(o) { const out = __c(this); _glMatrix._mat3.subtract(out.__arr, this.__arr, o.__arr); return out; }
    subEq(o) { _glMatrix._mat3.subtract(this.__arr, this.__arr, o.__arr); return this; }
    mul(o) { const out = __c(this); _glMatrix._mat3.multiply(out.__arr, this.__arr, o.__arr); return out; }
    mulEq(o) { _glMatrix._mat3.multiply(this.__arr, this.__arr, o.__arr); return this; }
    mulScalar(o) { const out = __c(this); _glMatrix._mat3.multiplyScalar(out.__arr, this.__arr, o); return out; }
    mulScalarEq(o) { _glMatrix._mat3.multiplyScalar(this.__arr, this.__arr, o); return this; }
    equals(o) { return _glMatrix._mat3.equals(this.__arr, o.__arr); }
    equalsExact(o) { return _glMatrix._mat3.equalsExact(this.__arr, o.__arr); }
    inverse() { const out = __c(this); _glMatrix._mat3.invert(out.__arr, this.__arr); return out; }
    invert() { _glMatrix._mat3.invert(this.__arr, this.__arr); return this; }
    
    rotated(angle) { const out = __c(this); _glMatrix._mat3.rotate(out.__arr, this.__arr, angle * DEG_TO_RAD); return out; }
    rotate(angle) { _glMatrix._mat3.rotate(this.__arr, this.__arr, angle * DEG_TO_RAD); return this; }
    scaled(s) { const out = __c(this); _glMatrix._mat3.scale(out.__arr, this.__arr, s.__arr); return out; }
    scale(s) { _glMatrix._mat3.scale(this.__arr, this.__arr, s.__arr); return this; }
    translated(t) { const out = __c(this); _glMatrix._mat3.translate(out.__arr, this.__arr, t.__arr); return out; }
    translate(t) { _glMatrix._mat3.translate(this.__arr, this.__arr, t.__arr); return this; }
    transposed() { const out = __c(this); _glMatrix._mat3.transpose(out.__arr, this.__arr); return out; }
    transpose() { _glMatrix._mat3.transpose(this.__arr, this.__arr); return this; }
    
    adjoint() { const out = __c(this); _glMatrix._mat3.adjoint(out.__arr, this.__arr); return out; }
    determinant() { return _glMatrix._mat3.determinant(this.__arr); }
    frob() { return _glMatrix._mat3.frob(this.__arr); }
    
    toString(digits = 2) {
        return `${this.__arr[0].toFixed(digits)}, ${this.__arr[1].toFixed(digits)}, ${this.__arr[2].toFixed(digits)}\n` +
               `${this.__arr[3].toFixed(digits)}, ${this.__arr[4].toFixed(digits)}, ${this.__arr[5].toFixed(digits)}\n` +
               `${this.__arr[6].toFixed(digits)}, ${this.__arr[7].toFixed(digits)}, ${this.__arr[8].toFixed(digits)}`;
    }
    
    get x() {
        if (!this.__x) this.__x = new vec3(this.__arr, { __view: [0, 3]});
        return this.__x;
    }
    set x(v3) {
        this.__arr[0] = v3.__arr[0];
        this.__arr[1] = v3.__arr[1];
        this.__arr[2] = v3.__arr[2];
    }
    get y() {
        if (!this.__y) this.__y = new vec3(this.__arr, { __view: [3, 3]});
        return this.__y;
    }
    set y(v3) {
        this.__arr[3] = v3.__arr[0];
        this.__arr[4] = v3.__arr[1];
        this.__arr[5] = v3.__arr[2];
    }
    get z() {
        if (!this.__z) this.__z = new vec3(this.__arr, { __view: [6, 3]});
        return this.__z;
    }
    set z(v3) {
        this.__arr[6] = v3.__arr[0];
        this.__arr[7] = v3.__arr[1];
        this.__arr[8] = v3.__arr[2];
    }
};

class mat2 {
    constructor(arr, args) {
        this.__arr = arr;
        this.__x = null;
        this.__y = null;
        __set_arr(this, args, 4, 'mat2*');
    }
    
    clone() { const out = __c(this); out.__arr.set(this.__arr); return out; }
    set() { return __set_arr(this, arguments, 4, 'mat2*'); }
    identity() { _glMatrix._mat2.identity(this.__arr); return this; }
    
    add(o) { const out = __c(this); _glMatrix._mat2.add(out.__arr, this.__arr, o.__arr); return out; }
    addEq(o) { _glMatrix._mat2.add(this.__arr, this.__arr, o.__arr); return this; }
    sub(o) { const out = __c(this); _glMatrix._mat2.subtract(out.__arr, this.__arr, o.__arr); return out; }
    subEq(o) { _glMatrix._mat2.subtract(this.__arr, this.__arr, o.__arr); return this; }
    mul(o) { const out = __c(this); _glMatrix._mat2.multiply(out.__arr, this.__arr, o.__arr); return out; }
    mulEq(o) { _glMatrix._mat2.multiply(this.__arr, this.__arr, o.__arr); return this; }
    mulScalar(o) { const out = __c(this); _glMatrix._mat2.multiplyScalar(out.__arr, this.__arr, o); return out; }
    mulScalarEq(o) { _glMatrix._mat2.multiplyScalar(this.__arr, this.__arr, o); return this; }
    equals(o) { return _glMatrix._mat2.equals(this.__arr, o.__arr); }
    equalsExact(o) { return _glMatrix._mat2.equalsExact(this.__arr, o.__arr); }
    inverse() { const out = __c(this); _glMatrix._mat2.invert(out.__arr, this.__arr); return out; }
    invert() { _glMatrix._mat2.invert(this.__arr, this.__arr); return this; }
    
    rotated(angle) { const out = __c(this); _glMatrix._mat2.rotate(out.__arr, this.__arr, angle * DEG_TO_RAD); return out; }
    rotate(angle) { _glMatrix._mat2.rotate(this._mat2, this.__arr, angle * DEG_TO_RAD); return this; }
    scaled(s) { const out = __c(this); _glMatrix._mat2.scale(out.__arr, this.__arr, s.__arr); return out; }
    scale(s) { _glMatrix._mat2.scale(this.__arr, this.__arr, s.__arr); return this; }
    transposed() { const out = __c(this); _glMatrix._mat2.transpose(out.__arr, this.__arr); return out; }
    transpose() { _glMatrix._mat2.transpose(this.__arr, this.__arr); return this; }
    
    adjoint() { const out = __c(this); _glMatrix._mat2.adjoint(out.__arr, this.__arr); return out; }
    determinant() { return _glMatrix._mat2.determinant(this.__arr); }
    frob() { return _glMatrix._mat2.frob(this.__arr); }
    
    toString(digits = 2) {
        return `${this.__arr[0].toFixed(digits)}, ${this.__arr[1].toFixed(digits)}\n${this.__arr[2].toFixed(digits)}, ${this.__arr[3].toFixed(digits)}`;
    }
    
    get x() {
        if (!this.__x) this.__x = new vec2(this.__arr, { __view: [0, 2]});
        return this.__x;
    }
    set x(v2) {
        this.__arr[0] = v2.__arr[0];
        this.__arr[1] = v2.__arr[1];
    }
    get y() {
        if (!this.__y) this.__y = new vec2(this.__arr, { __view: [2, 2]});
        return this.__y;
    }
    set y(v2) {
        this.__arr[2] = v2.__arr[0];
        this.__arr[3] = v2.__arr[1];
    }
};

var vec4f  = class extends vec4 { constructor() { super(new Float32Array(4 ), arguments); } static random(length) { const out = new vec4f(); _glMatrix._vec4.random(out.__arr, length); return out; } };
var vec4i  = class extends vec4 { constructor() { super(new Int32Array  (4 ), arguments); } static random(length) { const out = new vec4i(); _glMatrix._vec4.random(out.__arr, length); return out; } };
var vec3f  = class extends vec3 { constructor() { super(new Float32Array(3 ), arguments); } static random(length) { const out = new vec3f(); _glMatrix._vec3.random(out.__arr, length); return out; } };
var vec3i  = class extends vec3 { constructor() { super(new Int32Array  (3 ), arguments); } static random(length) { const out = new vec3i(); _glMatrix._vec3.random(out.__arr, length); return out; } };
var vec2f  = class extends vec2 { constructor() { super(new Float32Array(2 ), arguments); } static random(length) { const out = new vec2f(); _glMatrix._vec2.random(out.__arr, length); return out; } };
var vec2i  = class extends vec2 { constructor() { super(new Int32Array  (2 ), arguments); } static random(length) { const out = new vec2i(); _glMatrix._vec2.random(out.__arr, length); return out; } };
var vec4ui = class extends vec4 { constructor() { super(new Uint32Array (4 ), arguments); } };
var vec3ui = class extends vec3 { constructor() { super(new Uint32Array (3 ), arguments); } };
var vec2ui = class extends vec2 { constructor() { super(new Uint32Array (2 ), arguments); } };
var mat4i  = class extends mat4 { constructor() { super(new Int32Array  (16), arguments); } };
var mat4ui = class extends mat4 { constructor() { super(new Uint32Array (16), arguments); } };
var mat4f  = class extends mat4 { constructor() { super(new Float32Array(16), arguments); } };
var mat3i  = class extends mat3 { constructor() { super(new Int32Array  (9 ), arguments); } };
var mat3ui = class extends mat3 { constructor() { super(new Uint32Array (9 ), arguments); } };
var mat3f  = class extends mat3 { constructor() { super(new Float32Array(9 ), arguments); } };
var mat2i  = class extends mat2 { constructor() { super(new Int32Array  (4 ), arguments); } };
var mat2ui = class extends mat2 { constructor() { super(new Uint32Array (4 ), arguments); } };
var mat2f  = class extends mat2 { constructor() { super(new Float32Array(4 ), arguments); } };

var Transform3D = {
    rotationTranslation: (r, t) => { const out = new mat4f(); _glMatrix._mat4.fromRotationTranslation(out.__arr, r.__arr, t.__arr); return out; },
    rotationTranslationScale: (r, t, s) => { const out = new mat4f(); _glMatrix._mat4.fromRotationTranslationScale(out.__arr, r.__arr, t.__arr, s.__arr); return out; },
    rotationTranslationScaleOrigin: (r, t, s, o) => { const out = new mat4f(); _glMatrix._mat4.fromRotationTranslationScaleOrigin(out.__arr, r.__arr, t.__arr, s.__arr, o.__arr); return out; },
    rotationX: (angle) => { const out = new mat4f(); _glMatrix._mat4.fromXRotation(out.__arr, angle * DEG_TO_RAD); return out; },
    rotationY: (angle) => { const out = new mat4f(); _glMatrix._mat4.fromYRotation(out.__arr, angle * DEG_TO_RAD); return out; },
    rotationZ: (angle) => { const out = new mat4f(); _glMatrix._mat4.fromZRotation(out.__arr, angle * DEG_TO_RAD); return out; },
    rotation: (axisOrQuat, angle) => {
        const out = new mat4f(); 
        if (axisOrQuat instanceof quat) _glMatrix._mat4.fromQuat(out.__arr, axisOrQuat.__arr);
        else if (axisOrQuat instanceof vec3 && (angle !== null && angle !== undefined)) _glMatrix._mat4.fromRotation(out.__arr, angle * DEG_TO_RAD, axisOrQuat.__arr);
        else engine.error('Invalid arguments passed to Transform.rotation. Must either be (instance of quat), or (instance of vec3*, angle).');
        return out;
    },
    translation: (t) => { const out = new mat4f(); _glMatrix._mat4.fromTranslation(out.__arr, t.__arr); return out; },
    scale: (s) => { const out = new mat4f(); _glMatrix._mat4.fromScaling(out.__arr, s.__arr); return out; },
    targetTo: (eye, center, up) => { const out = new mat4f(); _glMatrix._mat4.targetTo(out.__arr, eye.__arr, center.__arr, up.__arr); }
};

var Transform2D = {
    rotation: (quatOrAngle) => {
        const out = new mat3f();
        if (quatOrAngle instanceof quat) _glMatrix._mat3.fromQuat(out.__arr, quatOrAngle.__arr);
        else _glMatrix._mat3.fromRotation(out.__arr, angle * DEG_TO_RAD);
        return out;
    },
    translation: (t) => { const out = new mat3f(); _glMatrix._mat3.fromTranslation(out.__arr, t.__arr); return out; },
    scale: (s) => { const out = new mat3f(); _glMatrix._mat3.fromScaling(out.__arr, s.__arr); return out; },
};

var View = {
    lookAt: (eye, center, up) => { const out = new mat4f(); _glMatrix._mat4.lookAt(out.__arr, eye.__arr, center.__arr, up.__arr); return out; }
};

var Projection = {
    frustum: (left, right, bottom, top, near, far) => { const out = new mat4f(); _glMatrix._mat4.frustum(out.__arr, left, right, bottom, top, near, far); return out; },
    ortho: (left, right, bottom, top, near, far) => { const out = new mat4f(); _glMatrix._mat4.ortho(out.__arr, left, right, bottom, top, near, far); return out; },
    perspective: (fov, aspect, near, far) => { const out = new mat4f(); _glMatrix._mat4.perspective(out.__arr, fov * DEG_TO_RAD, aspect, near, far); return out; },
    perspectiveFromFieldOfView: (fovUp, fovDown, fovLeft, fovRight, near, far) => {
        const out = mat4f();
        // this function actually does use degrees, for some reason.
        // If the user specified they will be using radians, convert
        // to degrees.
        const mult = DEG_TO_RAD === 1.0 ? 57.2957795131 : DEG_TO_RAD;
        const fov = {
            upDegrees: fovUp * mult,
            downDegrees: fovDown * mult,
            leftDegrees: fovLeft * mult,
            rightDegrees: fovRight * mult
        };
        _glMatrix._mat4.perspectiveFromFieldOfView(out.__arr, fov, near, far);
        return out;
    }
};

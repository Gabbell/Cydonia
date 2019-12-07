struct TimeInSeconds
{
    float iTime;
};

uniform TimeInSeconds _122;

layout(location = 0) out vec4 outColor;
float prm1;
vec2 bsMo;

vec2 disp(float t)
{
    return vec2(sin(t * 0.2199999988079071044921875) * 1.0, cos(t * 0.17499999701976776123046875) * 1.0) * 2.0;
}

mat2 rot(float a)
{
    float c = cos(a);
    float s = sin(a);
    return mat2(vec2(c, s), vec2(-s, c));
}

float mag2(vec2 p)
{
    return dot(p, p);
}

vec2 map(inout vec3 p)
{
    vec3 p2 = p;
    float param = p.z;
    vec2 _115 = p2.xy - disp(param);
    p2 = vec3(_115.x, _115.y, p2.z);
    float param_1 = (sin(p.z + _122.iTime) * (0.100000001490116119384765625 + (prm1 * 0.0500000007450580596923828125))) + (_122.iTime * 0.0900000035762786865234375);
    vec2 _145 = p.xy * rot(param_1);
    p = vec3(_145.x, _145.y, p.z);
    vec2 param_2 = p2.xy;
    float cl = mag2(param_2);
    float d = 0.0;
    p *= 0.61000001430511474609375;
    float z = 1.0;
    float trk = 1.0;
    float dspAmp = 0.100000001490116119384765625 + (prm1 * 0.20000000298023223876953125);
    for (int i = 0; i < 5; i++)
    {
        p += (sin(((p.zxy * 0.75) * trk) + vec3((_122.iTime * trk) * 0.800000011920928955078125)) * dspAmp);
        d -= abs(dot(cos(p), sin(p.yzx)) * z);
        z *= 0.569999992847442626953125;
        trk *= 1.39999997615814208984375;
        p *= mat3(vec3(0.643423378467559814453125, 1.08145618438720703125, -1.3860681056976318359375), vec3(-1.69621908664703369140625, 0.630164325237274169921875, -0.2957338988780975341796875), vec3(0.2926265895366668701171875, 1.3432028293609619140625, 1.18384265899658203125));
    }
    d = ((abs(d + (prm1 * 3.0)) + (prm1 * 0.300000011920928955078125)) - 2.5) + bsMo.y;
    return vec2((d + (cl * 0.20000000298023223876953125)) + 0.25, cl);
}

float linstep(float mn, float mx, float x)
{
    return clamp((x - mn) / (mx - mn), 0.0, 1.0);
}

vec4 render(vec3 ro, vec3 rd, float time)
{
    vec4 rez = vec4(0.0);
    float param = time + 8.0;
    vec3 lpos = vec3(disp(param) * 0.5, time + 8.0);
    float t = 1.5;
    float fogT = 0.0;
    for (int i = 0; i < 130; i++)
    {
        if (rez.w > 0.9900000095367431640625)
        {
            break;
        }
        vec3 pos = ro + (rd * t);
        vec3 param_1 = pos;
        vec2 _301 = map(param_1);
        vec2 mpv = _301;
        float den = clamp(mpv.x - 0.300000011920928955078125, 0.0, 1.0) * 1.12000000476837158203125;
        float dn = clamp(mpv.x + 2.0, 0.0, 3.0);
        vec4 col = vec4(0.0);
        if (mpv.x > 0.60000002384185791015625)
        {
            col = vec4((sin(((vec3(5.0, 0.4000000059604644775390625, 0.20000000298023223876953125) + vec3(mpv.y * 0.100000001490116119384765625)) + vec3(sin(pos.z * 0.4000000059604644775390625) * 0.5)) + vec3(1.7999999523162841796875)) * 0.5) + vec3(0.5), 0.07999999821186065673828125);
            col *= ((den * den) * den);
            float param_2 = 4.0;
            float param_3 = -2.5;
            float param_4 = mpv.x;
            vec3 _368 = col.xyz * (linstep(param_2, param_3, param_4) * 2.2999999523162841796875);
            col = vec4(_368.x, _368.y, _368.z, col.w);
            vec3 param_5 = pos + vec3(0.800000011920928955078125);
            vec2 _377 = map(param_5);
            float dif = clamp((den - _377.x) / 9.0, 0.001000000047497451305389404296875, 1.0);
            vec3 param_6 = pos + vec3(0.3499999940395355224609375);
            vec2 _390 = map(param_6);
            dif += clamp((den - _390.x) / 2.5, 0.001000000047497451305389404296875, 1.0);
            vec3 _411 = col.xyz * ((vec3(0.004999999888241291046142578125, 0.04500000178813934326171875, 0.07500000298023223876953125) + (vec3(0.0494999997317790985107421875, 0.104999996721744537353515625, 0.04500000178813934326171875) * dif)) * den);
            col = vec4(_411.x, _411.y, _411.z, col.w);
        }
        float fogC = exp((t * 0.20000000298023223876953125) - 2.2000000476837158203125);
        col += (vec4(0.0599999986588954925537109375, 0.10999999940395355224609375, 0.10999999940395355224609375, 0.100000001490116119384765625) * clamp(fogC - fogT, 0.0, 1.0));
        fogT = fogC;
        rez += (col * (1.0 - rez.w));
        t += clamp(0.5 - ((dn * dn) * 0.0500000007450580596923828125), 0.0900000035762786865234375, 0.300000011920928955078125);
    }
    return clamp(rez, vec4(0.0), vec4(1.0));
}

float getsat(vec3 c)
{
    float mi = min(min(c.x, c.y), c.z);
    float ma = max(max(c.x, c.y), c.z);
    return (ma - mi) / (ma + 1.0000000116860974230803549289703e-07);
}

vec3 iLerp(vec3 a, vec3 b, float x)
{
    vec3 ic = mix(a, b, vec3(x)) + vec3(9.9999999747524270787835121154785e-07, 0.0, 0.0);
    vec3 param = ic;
    vec3 param_1 = a;
    vec3 param_2 = b;
    float sd = abs(getsat(param) - mix(getsat(param_1), getsat(param_2), x));
    vec3 dir = normalize(vec3(((2.0 * ic.x) - ic.y) - ic.z, ((2.0 * ic.y) - ic.x) - ic.z, ((2.0 * ic.z) - ic.y) - ic.x));
    float lgt = dot(vec3(1.0), ic);
    float ff = dot(dir, normalize(ic));
    ic += ((((dir * 1.5) * sd) * ff) * lgt);
    return clamp(ic, vec3(0.0), vec3(1.0));
}

void main()
{
    prm1 = 0.0;
    bsMo = vec2(0.0);
    vec2 iResolution = vec2(1280.0, 720.0);
    vec2 iMouse = vec2(0.0);
    vec2 q = gl_FragCoord.xy / iResolution;
    vec2 p = (gl_FragCoord.xy - (iResolution * 0.5)) / vec2(iResolution.y);
    bsMo = (iMouse - (iResolution * 0.5)) / vec2(iResolution.y);
    float time = _122.iTime * 3.0;
    vec3 ro = vec3(0.0, 0.0, time);
    ro += vec3(sin(_122.iTime) * 0.5, sin(_122.iTime * 1.0) * 0.0, 0.0);
    float dspAmp = 0.85000002384185791015625;
    float param = ro.z;
    vec2 _618 = ro.xy + (disp(param) * dspAmp);
    ro = vec3(_618.x, _618.y, ro.z);
    float tgtDst = 3.5;
    float param_1 = time + tgtDst;
    vec3 target = normalize(ro - vec3(disp(param_1) * dspAmp, time + tgtDst));
    ro.x -= (bsMo.x * 2.0);
    vec3 rightdir = normalize(cross(target, vec3(0.0, 1.0, 0.0)));
    vec3 updir = normalize(cross(rightdir, target));
    rightdir = normalize(cross(updir, target));
    vec3 rd = normalize((((rightdir * p.x) + (updir * p.y)) * 1.0) - target);
    float param_2 = time + 3.5;
    float param_3 = ((-disp(param_2).x) * 0.20000000298023223876953125) + bsMo.x;
    vec2 _689 = rd.xy * rot(param_3);
    rd = vec3(_689.x, _689.y, rd.z);
    prm1 = smoothstep(-0.4000000059604644775390625, 0.4000000059604644775390625, sin(_122.iTime * 0.300000011920928955078125));
    vec3 param_4 = ro;
    vec3 param_5 = rd;
    float param_6 = time;
    vec4 scn = render(param_4, param_5, param_6);
    vec3 col = scn.xyz;
    vec3 param_7 = col.zyx;
    vec3 param_8 = col;
    float param_9 = clamp(1.0 - prm1, 0.0500000007450580596923828125, 1.0);
    col = iLerp(param_7, param_8, param_9);
    col = pow(col, vec3(0.550000011920928955078125, 0.64999997615814208984375, 0.60000002384185791015625)) * vec3(1.0, 0.9700000286102294921875, 0.89999997615814208984375);
    col *= ((pow((((16.0 * q.x) * q.y) * (1.0 - q.x)) * (1.0 - q.y), 0.119999997317790985107421875) * 0.699999988079071044921875) + 0.300000011920928955078125);
    outColor = vec4(col, 1.0);
}


#!/usr/bin/lua

function J0(x)
    -- First kind Bessel function
    -- of the zero alpha value approximation
    return (1.0 - ((x^2) / 2.0))
end

function J1(x)
    -- First kind bessel function
    -- of the one alpha value approximation
    return (x / 2.0)
end

print()

N = 12
T = (N / 1000.0)
t = (N / 100000.0)
dt = (t * 0.8)
W = (2.0 * math.pi * 200.0 * N)
print(string.format("N          = % d", N))
print(string.format("T          = % f", T))
print(string.format("τ          = % f", t))
print(string.format("Δτ         = % f", dt))
print(string.format("Ω          = % f", W))

U_0 = (1.0)
w = (2.0 * math.pi / T)
print(string.format("U_0        = % f", U_0))
print(string.format("ω          = % f", w))
print()

wl = (w - W)
wr = (w + W)
U_in_W = (U_0 * dt / T)
U_in_w = (2.0 * U_0 * t / T) * (math.sin(w * t / 2.0) / (w * t / 2.0)) * J0(w * dt / 2.0)
U_in_l = (2.0 * U_0 * t / T) * (math.sin(wl * t / 2.0) / (wl * t / 2.0)) * J1(w * dt / 2.0)
U_in_r = (2.0 * U_0 * t / T) * (math.sin(wr * t / 2.0) / (wr * t / 2.0)) * J1(w * dt / 2.0)
print(string.format("U_in_Ω     = % f", U_in_W))
print(string.format("U_in_ω     = % f", U_in_w))
print(string.format("U_in_ω-Ω   = % f", U_in_l))
print(string.format("U_in_ω+Ω   = % f", U_in_r))
print()

t_cut = (1.0 / W)
K_w = (1.0 / math.sqrt(1.0 + (w^4 * t_cut^4)))
K_W = (1.0 / math.sqrt(1.0 + (W^4 * t_cut^4)))
K_l = (1.0 / math.sqrt(1.0 + (wl^4 * t_cut^4)))
K_r = (1.0 / math.sqrt(1.0 + (wr^4 * t_cut^4)))
print(string.format("t_cut      = % f", t_cut))
print(string.format("K_ω        = % f dB (% f)", 20.0 * math.log10(K_w), K_w))
print(string.format("K_Ω        = % f dB (% f)", 20.0 * math.log10(K_W), K_W))
print(string.format("K_ω-Ω      = % f dB (% f)", 20.0 * math.log10(K_l), K_l))
print(string.format("K_ω+Ω      = % f dB (% f)", 20.0 * math.log10(K_r), K_r))
print()

U_out_w = U_in_w * K_w
U_out_W = U_in_W * K_W
U_out_l = U_in_l * K_l
U_out_r = U_in_r * K_r
print(string.format("U_out_ω    = % f", U_out_w))
print(string.format("U_out_Ω    = % f", U_out_W))
print(string.format("U_out_ω-Ω  = % f", U_out_l))
print(string.format("U_out_ω+Ω  = % f", U_out_r))
print()

delta = math.sqrt((U_out_w^2) + (U_out_l^2) + (U_out_r^2)) / U_out_W
print(string.format("δ          = % f", delta))
print()

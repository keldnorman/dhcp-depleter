#!/bin/bash
clear
# set -x
set -euo pipefail
#---------------------------------------------------------------------------------------
# About
#---------------------------------------------------------------------------------------
# DHCP Wi-Fi test for Linux. Menu UI. No web server.
# Unmanages IF via NetworkManager, associates, randomizes MAC, requests DHCP, repeats.
#---------------------------------------------------------------------------------------
# Banners for the 1337'ishness
#---------------------------------------------------------------------------------------
cat << "EOF"
      ____
 ____|    \
(____|     `._____
 ____|       _|___ D H C P - D e p l e t e r . . . 
(____|     .'
     |____/   Test tool by Keld Norman (C)opyleft 2025
EOF
#---------------------------------------------------------------------------------------
# Privilege handling (silent sudo if possible)
#---------------------------------------------------------------------------------------
if [[ $EUID -ne 0 ]]; then
  if command -v sudo >/dev/null 2>&1; then
    if sudo -n true 2>/dev/null; then exec sudo -- "$0" "$@"; else
      echo "[+] This script needs root. The upcoming prompt is sudo."
      exec sudo -- "$0" "$@"
    fi
  else
    echo "[!] sudo not found. Run as root."; exit 1
  fi
fi
#---------------------------------------------------------------------------------------
# Variables
#---------------------------------------------------------------------------------------
ASSOC_TIMEOUT=7
CARRIER_WAIT_SEC=0.25
DHCP_TIMEOUT=10
MIN_PAUSE_SEC=0.08
#---------------------------------------------------------------------------------------
# Globals
#---------------------------------------------------------------------------------------
IFACE=""
declare -a SELECTED_IFACES=()
declare -A NM_WAS_MANAGED_MAP=()
NM_PRESENT=0
WPA_PID=""
DHCLIENT_RUNNING=0
ONLY_OPEN=0
CLEANED=0
EXIT_STATUS=0
#---------------------------------------------------------------------------------------
# Check for dependencies
#---------------------------------------------------------------------------------------
need(){ command -v "$1" >/dev/null || { echo "[!] Missing: $1"; exit 1; }; }
need nmcli; need iw; need ip; need awk; need sed; need timeout; need od; need tr; need grep; need cut
need wpa_supplicant; need wpa_passphrase
if command -v dhclient >/dev/null 2>&1; then USE_DHCP="dhclient"
elif command -v udhcpc >/dev/null 2>&1; then USE_DHCP="udhcpc"
else echo "[!] Missing DHCP client: install dhclient or busybox udhcpc"; exit 1; fi
#---------------------------------------------------------------------------------------
# ANSI colors
#---------------------------------------------------------------------------------------
if [[ -n "${NO_COLOR:-}" ]]; then CRED=""; CGRN=""; CYEL=""; C0=""
else CRED=$'\e[31m'; CGRN=$'\e[32m'; CYEL=$'\e[33m'; C0=$'\e[0m'; fi
color_for_dbm(){ local d="${1:-}"; [[ "$d" =~ ^-?[0-9]+$ ]] || { echo ""; return; }
  if   (( d >= -60 )); then echo "$CGRN"
  elif (( d >= -75 )); then echo "$CYEL"
  else                    echo "$CRED"
 fi
}
#---------------------------------------------------------------------------------------
# Hostname Themes
#---------------------------------------------------------------------------------------
PREFIXES=(PC LAPTOP DESKTOP WIN10 WIN11 CORP DEV HR OFFICE)
THEME=IT
CUSTOM_BASE=""
CUSTOM_SEQ=1
THEME_IT=(BSOD PANIC OOPS LOLCAT OWNED ERROR PWND HACKZ PROXY SUDO NULL WORM)
THEME_FOOD=(PIZZA TACO SUSHI KEBAB PASTA DONUT COCOA LATTE CIDER BEER RUM GIN)
THEME_GAMES=(MARIO LINK ZELDA SONIC DOOM PONG TETRIS PACMAN QUAKE DIABLO LARA)
THEME_SPACE=(MARS LUNA JUNO TITAN IO VULCAN WARP BORG SPOCK YODA DROID ALIEN)
THEME_FUNNY=(DERP LOL OMG WTF MEH ROFL YEET SUS UWU PEPE TROLL MEME)
THEME_MUSIC=(ELVIS QUEEN ACDC METAL JAZZ FUNK BLUES OPERA DISCO EMO RAPPER)
THEME_NATURE=(FIRE LAVA WIND RAIN SNOW ICE ROCK TREE LEAF ROOT SAND WAVE STAR)
THEME_ANIMALS=(CAT DOG FOX BAT OWL ELK LION BEAR DEER SWAN DUCK CRAB TUNA SEAL CROW)
rand16(){ od -An -N2 -tu2 /dev/urandom 2>/dev/null | tr -d ' ' || echo $((RANDOM&0xFFFF)); }
pick(){ local -n arr=$1; local n=${#arr[@]}; echo "${arr[$(( $(rand16) % n ))]}"; }
sanitize_base(){ echo "$1" | tr -cd 'A-Za-z0-9_-' | cut -c1-10; }
make_hostname(){
  if [[ "$THEME" == "CUSTOM" ]]; then
    local base="$(sanitize_base "${CUSTOM_BASE:-HOST}")"
    printf "%s-%d" "${base:-HOST}" "$CUSTOM_SEQ"
    CUSTOM_SEQ=$((CUSTOM_SEQ>=9999999999 ? 1 : CUSTOM_SEQ+1)); return
  fi
  local word p num
  case "$THEME" in
    IT) word="$(pick THEME_IT)";; FOOD) word="$(pick THEME_FOOD)";;
    GAMES) word="$(pick THEME_GAMES)";; SPACE) word="$(pick THEME_SPACE)";;
    FUNNY) word="$(pick THEME_FUNNY)";; MUSIC) word="$(pick THEME_MUSIC)";;
    NATURE) word="$(pick THEME_NATURE)";; ANIMALS) word="$(pick THEME_ANIMALS)";;
    *) word="$(pick THEME_IT)";;
  esac
  p="$(pick PREFIXES)"; num=$(( ( $(rand16) % 99 ) + 1 ))
  printf "%s-%s%02d" "$p" "$word" "$num"
}
#---------------------------------------------------------------------------------------
# Helpers
#---------------------------------------------------------------------------------------
random_mac6(){ od -An -N6 -tx1 /dev/urandom | tr -d ' \n'; }
random_laa_mac(){ local hex; hex="$(random_mac6)"; local b1=$(( 0x${hex:0:2} ))
  b1=$(( (b1 & 0xFC) | 0x02 )); printf "%02x:%s:%s:%s:%s:%s\n" "$b1" "${hex:2:2}" "${hex:4:2}" "${hex:6:2}" "${hex:8:2}" "${hex:10:2}"
}
iface_down_up_with_mac(){ local dev="$1" mac="$2"
  ip link set "$dev" down
  ip link set "$dev" address "$mac"
  ip link set "$dev" up
  sleep 0.06
}
get_current_rssi_dbm(){ local v; v="$(iw dev "$IFACE" link 2>/dev/null | awk '/signal:/ {print $2; exit}')"
  [[ "$v" =~ ^-?[0-9]+$ ]] && { echo "$v"; return; }; echo ""
}
fmt_chan(){ local c="$1"; [[ "$c" =~ ^[0-9]+$ ]] && printf "CH%03d" "$((10#$c))" || printf "CH???"; }
iface_ipv4(){ ip -4 addr show dev "$1" | awk '/inet /{print $2}' | cut -d/ -f1 | head -n1; }

# bands: wiphy → phyN info; fallback via sysfs
iface_bands(){
  local ifc="$1" wiphy has24=0 has5=0 info
  wiphy="$(iw dev "$ifc" info 2>/dev/null | awk '/wiphy/ {print $2; exit}')"
  if [[ -n "$wiphy" ]]; then
    info="$(iw phy "phy$wiphy" info 2>/dev/null || true)"
  else
    local phy; phy="$(basename "$(readlink -f "/sys/class/net/$ifc/phy80211" 2>/dev/null)" 2>/dev/null || true)"
    [[ -n "$phy" ]] && info="$(iw phy "$phy" info 2>/dev/null || true)"
  fi
  grep -q '2412 MHz' <<<"${info:-}" && has24=1
  grep -q '5180 MHz' <<<"${info:-}" && has5=1
  if   ((has24&&has5)); then echo "2.4G/5G"
  elif ((has24));      then echo "2.4G"
  elif ((has5));       then echo "5G"
  else echo "?"; fi
}
#---------------------------------------------------------------------------------------
# Trap cleanup 
#---------------------------------------------------------------------------------------
cleanup(){
  (( CLEANED == 1 )) && exit "${EXIT_STATUS:-$?}"
  CLEANED=1
  local s="${EXIT_STATUS:-$?}"
  # Release DHCP and stop wpa on all selected ifaces
  for ifc in "${SELECTED_IFACES[@]:-}"; do
    if [[ "$USE_DHCP" == "dhclient" ]]; then
      /sbin/dhclient -r -pf "/run/dhcp-deplete.${ifc}.pid" -lf "/tmp/dhcp-deplete.${ifc}.lease" "$ifc" >/dev/null 2>&1 || true
    else
      udhcpc -i "$ifc" -q -R >/dev/null 2>&1 || true
    fi
    pkill -f "wpa_supplicant.* -i $ifc" >/dev/null 2>&1 || true
  done
  # Restore NetworkManager management state
  if (( NM_PRESENT==1 )); then
    for ifc in "${SELECTED_IFACES[@]:-}"; do
      local was="${NM_WAS_MANAGED_MAP[$ifc]:-}"
      [[ -n "$was" ]] && nmcli dev set "$ifc" managed "$was" >/dev/null 2>&1 || true
    done
  fi
  echo
  echo "[+] Program stopped"
  echo
  exit "$s"
}
on_int(){ EXIT_STATUS=130; exit 130; }
trap on_int INT TERM
trap cleanup EXIT
#---------------------------------------------------------------------------------------
# Interface discovery and selection (multi)
#---------------------------------------------------------------------------------------
discover_ifaces(){ iw dev | awk '/Interface/ {print $2}'; }
choose_ifaces(){
  local arr=() i=0
  while read -r x; do [[ -n "$x" ]] && arr+=("$x"); done < <(discover_ifaces)
  local n=${#arr[@]}
  if (( n==0 )); then echo "[!] No Wi-Fi interfaces found."; exit 1
  elif (( n==1 )); then SELECTED_IFACES=("${arr[0]}"); IFACE="${arr[0]}"; return
  else
    echo
    echo "[+] Select netcard(s) to test with"
    echo ""
    printf "%-4s  %-12s  %-8s  %-15s\n" "Num" "Interface" "Bands" "IPv4"
    printf "%-4s  %-12s  %-8s  %-15s\n" "----" "------------" "--------" "---------------"
    for ((i=0;i<n;i++)); do
      local ipv4 bands; ipv4="$(iface_ipv4 "${arr[$i]}")"; bands="$(iface_bands "${arr[$i]}")"
      if [[ -n "$ipv4" ]]; then
        printf "${CRED}%4d  %-12s  %-8s  %-15s${C0}\n" "$((i+1))" "${arr[$i]}" "$bands" "$ipv4"
      else
        printf "${CGRN}%4d  %-12s  %-8s  %-15s${C0}\n" "$((i+1))" "${arr[$i]}" "$bands" "$ipv4"
      fi
    done
    echo ""
    read -r -p "[?] Pick number(s) (e.g. 1,3 or A=all): " pick
    pick="${pick^^}"
    if [[ "$pick" == "A" || "$pick" == "ALL" ]]; then
      SELECTED_IFACES=("${arr[@]}")
    else
      # split by comma or space
      IFS=', ' read -r -a sel <<<"$pick"
      local chosen=()
      for s in "${sel[@]}"; do
        [[ -z "$s" ]] && continue
        [[ "$s" =~ ^[0-9]+$ ]] || { echo "[!] Not a number: $s"; exit 1; }
        s=$((10#$s))
        (( s>=1 && s<=n )) || { echo "[!] Out of range: $s"; exit 1; }
        chosen+=("${arr[$((s-1))]}")
      done
      # dedup
      declare -A seen=(); SELECTED_IFACES=()
      for v in "${chosen[@]}"; do
        [[ -n "${seen[$v]:-}" ]] || { SELECTED_IFACES+=("$v"); seen[$v]=1; }
      done
      (( ${#SELECTED_IFACES[@]}>=1 )) || { echo "[!] No selection"; exit 1; }
    fi
    IFACE="${SELECTED_IFACES[0]}"
  fi
}
#---------------------------------------------------------------------------------------
# Theme menu + filter
#---------------------------------------------------------------------------------------
theme_menu(){
  echo ""
  echo "[+] Select hostname theme:"
  echo ""
  echo "  1) IT"
  echo "  2) FOOD"
  echo "  3) GAMES"
  echo "  4) SPACE"
  echo "  5) FUNNY"
  echo "  6) MUSIC"
  echo "  7) NATURE"
  echo "  8) ANIMALS"
  echo "  9) CUSTOM"
  echo ""
  read -r -p "[?] Enter number [1-9]: " t
  case "$t" in
    1) THEME=IT;;2) THEME=FOOD;;3) THEME=GAMES;;4) THEME=SPACE;;
    5) THEME=FUNNY;;6) THEME=MUSIC;;7) THEME=NATURE;;8) THEME=ANIMALS;;
    9) THEME=CUSTOM;;*) THEME=IT;;
  esac
  if [[ "$THEME" == "CUSTOM" ]]; then
    read -r -p "[?] Custom base prefix [3–10 A-Za-z0-9_-]: " CUSTOM_BASE
    CUSTOM_BASE="$(sanitize_base "$CUSTOM_BASE")"; [[ -z "$CUSTOM_BASE" ]] && CUSTOM_BASE="HOST"
    read -r -p "[?] Starting sequence number [default 1]: " s; s="${s:-1}"
    [[ "$s" =~ ^[0-9]+$ ]] && CUSTOM_SEQ="$s" || CUSTOM_SEQ=1
    echo
  fi
  read -r -n1 -p "[+] Do you have a passphrase ? [y/N] " a
  if [[ "${a:-N}" =~ ^[Yy]$ ]]; then ONLY_OPEN=0; else ONLY_OPEN=1; fi
  echo
  echo
}
#---------------------------------------------------------------------------------------
# Wireless SSID Scan
#---------------------------------------------------------------------------------------
SSIDS=(); BSSIDS=(); SIGS=(); SECS=(); CHANS=(); FREQS=(); ORDER=()
escape_placeholders(){ local s="$1"; local C=$'\x1f' B=$'\x1e'; s="${s//\\\\/$B}"; s="${s//\\:/$C}"; printf '%s' "$s"; }
unescape_placeholders(){ local s="$1"; local C=$'\x1f' B=$'\x1e'; s="${s//"$B"/\\}"; s="${s//"$C"/:}"; printf '%s' "$s"; }
declare -A RSSI_MAP=()
refresh_rssi_map(){ RSSI_MAP=()
  iw dev "$IFACE" scan 2>/dev/null | awk '
    /^BSS [0-9a-f:]+/ { gsub(/\(/,""); bssid=$2 }
    /signal:/ { dbm=$2; sub(/\..*$/,"",dbm); if (bssid!="") print bssid, dbm }
  ' | while read -r b d; do RSSI_MAP["$b"]="$d"; done
}
build_sort_order(){ ORDER=()
  while IFS=$'\t' read -r sig idx; do ORDER+=("$idx"); done < <(
    for ((i=0;i<${#SIGS[@]};i++)); do s="${SIGS[$i]}"; [[ "$s" =~ ^-?[0-9]+$ ]] || s="-100"; printf "%d\t%d\n" "$s" "$i"; done \
    | sort -nr -k1,1
  )
}
fmt_scan_header(){
  printf "\n%-4s  %-32s %-17s %10s %-7s %-12s\n" "Num" "SSID" "BSSID" "Signal" "CH" "SECURITY"
  printf "%-4s  %-32s %-17s %10s %-7s %-12s\n" "----" "--------------------------------" "-----------------" "----------" "-------" "------------"
}
is_open_sec(){ local s="$1"; [[ -z "$s" || "$s" == "--" || "$s" == "OPEN" || "$s" == "NONE" ]] && return 0 || return 1; }
scan_wifi(){
  SSIDS=(); BSSIDS=(); SIGS=(); SECS=(); CHANS=(); FREQS=(); ORDER=()
  echo "[+] Scanning Wi-Fi…"
  nmcli dev wifi rescan ifname "$IFACE" >/dev/null 2>&1 || true
  refresh_rssi_map
  while IFS= read -r raw; do
    [[ -z "$raw" ]] && continue
    escd="$(escape_placeholders "$raw")"
    IFS=':' read -r f1 f2 f3 f4 f5 f6 <<<"$escd"
    ssid="$(unescape_placeholders "${f1:-}")"; bssid="$(unescape_placeholders "${f2:-}")"
    sigpct="$(unescape_placeholders "${f3:-}")"; sec="$(unescape_placeholders "${f4:-}")"
    chan="$(unescape_placeholders "${f5:-}")"; freq="$(unescape_placeholders "${f6:-}")"
    [[ -z "${ssid}${bssid}" ]] && continue
    [[ -z "$ssid" ]] && ssid="<hidden>"
    if (( ONLY_OPEN==1 )) && ! is_open_sec "$sec"; then continue; fi
    local dbm
    if [[ -n "${RSSI_MAP[$bssid]:-}" ]]; then dbm="${RSSI_MAP[$bssid]}"
    elif [[ "$sigpct" =~ ^[0-9]+$ ]]; then dbm=$(( sigpct/2 - 100 ))
    else dbm="-100"; fi
    SSIDS+=("$ssid"); BSSIDS+=("$bssid"); SIGS+=("$dbm"); SECS+=("${sec:-OPEN}"); CHANS+=("${chan:-?}"); FREQS+=("${freq:-?}")
  done < <(nmcli -t --escape yes -f SSID,BSSID,SIGNAL,SECURITY,CHAN,FREQ dev wifi list ifname "$IFACE" || true)
  [[ ${#SSIDS[@]} -eq 0 ]] && { echo "[!] No networks found"; return 1; }
  build_sort_order
  fmt_scan_header
  for ((r=0;r<${#ORDER[@]};r++)); do
    local i="${ORDER[$r]}"; local CHDISP; CHDISP="$(fmt_chan "${CHANS[$i]}")"
    local d="${SIGS[$i]}"; local col; col="$(color_for_dbm "$d")"
    printf "${col}%04d  %-32.32s %-17s %6s dBm %-7s %-12s${C0}\n" \
      $((r+1)) "${SSIDS[$i]}" "${BSSIDS[$i]}" "$d" "$CHDISP" "${SECS[$i]}"
  done
}
#---------------------------------------------------------------------------------------
# Menu to pick Wireless SSID
#---------------------------------------------------------------------------------------
menu_pick(){
  while :; do
    echo
    read -r -p "[?] Enter number (R=rescan, Q=quit): " sel
    case "${sel^^}" in Q) exit 0 ;; R) scan_wifi || return 1; continue ;; esac
    [[ "$sel" =~ ^0*[0-9]+$ ]] || { echo "[!] Not a number"; continue; }
    local row=$((10#${sel}-1)); (( row>=0 && row<${#ORDER[@]} )) || { echo "[!] Out of range"; continue; }
    local idx="${ORDER[$row]}"
    SSID="${SSIDS[$idx]}"; BSSID="${BSSIDS[$idx]}"; SECURITY="${SECS[$idx]}"; CHAN="${CHANS[$idx]}"; SIG="${SIGS[$idx]}"
    local CHDISP; CHDISP="$(fmt_chan "$CHAN")"
    echo
    echo "[+] Selected: SSID=\"$SSID\" BSSID=$BSSID SEC=${SECURITY:-OPEN} ${CHDISP} SNR=${SIG} dBm"
    if (( ONLY_OPEN==1 )) || [[ -z "$SECURITY" || "$SECURITY" == "--" || "$SECURITY" == "OPEN" ]]; then PASSPHRASE=""
    else read -r -s -p "[?] Passphrase: " PASSPHRASE; echo
         (( ${#PASSPHRASE}>=8 && ${#PASSPHRASE}<=63 )) || { echo "[!] Passphrase must be 8–63"; continue; }
    fi
    return 0
  done
}
print_cycle_header(){
  printf "\n%-8s %-8s %-19s %-28s %-20s %-10s\n" "CYCLE" "IFACE" "MAC" "HOSTNAME" "IP" "RSSI"
  printf "%-8s %-8s %-19s %-28s %-20s %-10s\n" "-----" "-----" "-----------------" "----------------------------" "-------------------" "----------"
}
#---------------------------------------------------------------------------------------
# NetworkManager manage toggle for all selected IFs
#---------------------------------------------------------------------------------------
prepare_unmanaged_all(){
  if command -v nmcli >/dev/null 2>&1; then
    NM_PRESENT=1
    for ifc in "${SELECTED_IFACES[@]}"; do
      local was; was="$(nmcli -g GENERAL.MANAGED dev show "$ifc" 2>/dev/null | tr 'A-Z' 'a-z' || echo "")"
      [[ -z "$was" ]] && was="yes"
      NM_WAS_MANAGED_MAP[$ifc]="$was"
      nmcli dev disconnect "$ifc" >/dev/null 2>&1 || true
      nmcli dev set "$ifc" managed no >/dev/null 2>&1 || true
    done
  else
    NM_PRESENT=0
  fi
}
#---------------------------------------------------------------------------------------
# Association + DHCP without NM
#---------------------------------------------------------------------------------------
WPA_PID=""
stop_wpa(){ if [[ -n "${WPA_PID}" ]] && kill -0 "$WPA_PID" 2>/dev/null; then
  kill "$WPA_PID" >/dev/null 2>&1 || true; sleep 0.08; kill -9 "$WPA_PID" >/dev/null 2>&1 || true; WPA_PID=""
fi; }
post_assoc_ready(){
  sleep "$CARRIER_WAIT_SEC"
  ip link set "$IFACE" up >/dev/null 2>&1 || true
  ip addr flush dev "$IFACE" >/dev/null 2>&1 || true
  ip route flush dev "$IFACE" >/dev/null 2>&1 || true
}
associate_open(){
  local mac="$1" ssid="$2" bssid="$3"
  stop_wpa
  ip addr flush dev "$IFACE" >/dev/null 2>&1 || true
  ip route flush dev "$IFACE" >/dev/null 2>&1 || true
  iface_down_up_with_mac "$IFACE" "$mac"
  iw reg set DK >/dev/null 2>&1 || true
  iw dev "$IFACE" disconnect >/dev/null 2>&1 || true
  if [[ -n "$bssid" ]]; then
    timeout ${ASSOC_TIMEOUT}s iw dev "$IFACE" connect -w "$ssid" "$bssid" >/dev/null 2>&1
  else
    timeout ${ASSOC_TIMEOUT}s iw dev "$IFACE" connect -w "$ssid" >/dev/null 2>&1
  fi
  timeout ${ASSOC_TIMEOUT}s bash -c 'until iw dev '"$IFACE"' link | grep -q "^Connected to"; do sleep 0.1; done' >/dev/null 2>&1 || true
  post_assoc_ready
}
associate_wpa_psk(){
  local mac="$1" ssid="$2" psk="$3" bssid="$4"
  stop_wpa
  ip addr flush dev "$IFACE" >/dev/null 2>&1 || true
  ip route flush dev "$IFACE" >/dev/null 2>&1 || true
  iface_down_up_with_mac "$IFACE" "$mac"
  iw reg set DK >/dev/null 2>&1 || true
  iw dev "$IFACE" disconnect >/dev/null 2>&1 || true
  local cfg="/tmp/wpa_${IFACE}_$$.conf"
  wpa_passphrase "$ssid" "$psk" >"$cfg"
  if [[ -n "$bssid" ]]; then
    cat >>"$cfg" <<EOF

network={
    ssid="$ssid"
    psk="$psk"
    bssid=$bssid
    key_mgmt=WPA-PSK
}
EOF
  fi
  wpa_supplicant -B -i "$IFACE" -D nl80211 -c "$cfg" >/dev/null 2>&1
  WPA_PID="$(pgrep -n -f "wpa_supplicant -B -i $IFACE" || true)"
  timeout ${ASSOC_TIMEOUT}s bash -c 'until iw dev '"$IFACE"' link | grep -q "^Connected to"; do sleep 0.1; done' >/dev/null 2>&1 || true
  post_assoc_ready
}
set_transient_hostname(){ local hn="$1"
  if command -v hostnamectl >/dev/null 2>&1; then hostnamectl --transient set-hostname "$hn" >/dev/null 2>&1 || true
  else hostname "$hn" >/dev/null 2>&1 || true; fi
}
#---------------------------------------------------------------------------------------
# DHCP Request function
#---------------------------------------------------------------------------------------
dhcp_request(){
  local hn="$1"
  set_transient_hostname "$hn"
  ip addr flush dev "$IFACE" >/dev/null 2>&1 || true
  ip route flush dev "$IFACE" >/dev/null 2>&1 || true
  DHCLIENT_RUNNING=1
  if [[ "$USE_DHCP" == "udhcpc" ]]; then
    timeout ${DHCP_TIMEOUT}s udhcpc -i "$IFACE" -q -n -x hostname:"$hn" -T 2 -t 5 >/dev/null 2>&1
    return $?
  fi
  local dhcp_pid="/run/dhcp-deplete.${IFACE}.pid"
  local dhcp_lease="/tmp/dhcp-deplete.${IFACE}.lease"
  local dhcp_conf="/tmp/dhcp-deplete.${IFACE}.conf"
  pkill -f "dhclient.*\b$IFACE\b" >/dev/null 2>&1 || true
  rm -f "$dhcp_pid" "$dhcp_lease"
  printf 'send host-name "%s";\nrequest subnet-mask, routers, domain-name, domain-name-servers, host-name;\n' "$hn" > "$dhcp_conf"
  /sbin/dhclient -r -pf "$dhcp_pid" -lf "$dhcp_lease" -cf "$dhcp_conf" "$IFACE" >/dev/null 2>&1 || true
  timeout ${DHCP_TIMEOUT}s /sbin/dhclient -1 -4 -pf "$dhcp_pid" -lf "$dhcp_lease" -cf "$dhcp_conf" "$IFACE" >/dev/null 2>&1
}
#---------------------------------------------------------------------------------------
nm_connect_once_unmanaged(){
  local ssid="$1" bssid="$2" pass="$3"
  local mac hn rssi ip4
  mac="$(random_laa_mac)"; hn="$(make_hostname)"
  if [[ -z "$pass" ]]; then
    if ! associate_open "$mac" "$ssid" "$bssid"; then
      printf "%-8s %-8s %-19s %-28s %-20s %-10s\n" "FAIL" "$IFACE" "$mac" "$hn" "(assoc timeout)" ""
      return 1
    fi
  else
    if ! associate_wpa_psk "$mac" "$ssid" "$pass" "$bssid"; then
      printf "%-8s %-8s %-19s %-28s %-20s %-10s\n" "FAIL" "$IFACE" "$mac" "$hn" "(assoc timeout)" ""
      return 1
    fi
  fi
  if ! dhcp_request "$hn"; then
    rssi="$(get_current_rssi_dbm)"
    printf "%-8s %-8s %-19s %-28s %-20s %-10s\n" "FAIL" "$IFACE" "$mac" "$hn" "(DHCP timeout)" "${rssi:+$rssi dBm}"
    iw dev "$IFACE" disconnect >/dev/null 2>&1 || true
    stop_wpa
    return 1
  fi
  ip4="$(iface_ipv4 "$IFACE")"; rssi="$(get_current_rssi_dbm)"
  printf "%-8s %-8s %-19s %-28s %-20s %-10s\n" "OK" "$IFACE" "$mac" "$hn" "${ip4:-unknown}" "${rssi:+$rssi dBm}"
  if [[ "$USE_DHCP" == "dhclient" ]]; then
    /sbin/dhclient -r -pf "/run/dhcp-deplete.${IFACE}.pid" -lf "/tmp/dhcp-deplete.${IFACE}.lease" "$IFACE" >/dev/null 2>&1 || true
  else
    udhcpc -i "$IFACE" -q -R >/dev/null 2>&1 || true
  fi
  DHCLIENT_RUNNING=0
  ip addr flush dev "$IFACE" >/dev/null 2>&1 || true
  ip route flush dev "$IFACE" >/dev/null 2>&1 || true
  iw dev "$IFACE" disconnect >/dev/null 2>&1 || true
  stop_wpa
  return 0
}
#---------------------------------------------------------------------------------------
# Main
#---------------------------------------------------------------------------------------
choose_ifaces
theme_menu
# scan on the first selected iface
scan_wifi || exit 1
menu_pick
# Set all selected ifaces unmanaged during test
prepare_unmanaged_all
print_cycle_header
#---------------------------------------------------------------------------------------
# Run the test
#---------------------------------------------------------------------------------------
while :; do
  for ifc in "${SELECTED_IFACES[@]}"; do
    IFACE="$ifc"
    nm_connect_once_unmanaged "$SSID" "$BSSID" "${PASSPHRASE:-}" || true
    sleep "$MIN_PAUSE_SEC"
  done
done
#---------------------------------------------------------------------------------------
# End of script
#---------------------------------------------------------------------------------------


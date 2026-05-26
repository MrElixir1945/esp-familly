import os
import json
import logging
import requests
import asyncio
import pytz 
from datetime import datetime, time, timedelta
from dotenv import load_dotenv

from telegram import Update, InlineKeyboardButton, InlineKeyboardMarkup
from telegram.ext import ApplicationBuilder, CommandHandler, CallbackQueryHandler, MessageHandler, ContextTypes, filters, Application

# --- CONFIG ---
folder_path = os.path.dirname(os.path.abspath(__file__))
load_dotenv(os.path.join(folder_path, ".env"))

TOKEN = os.getenv("TELEGRAM_TOKEN")
ESP_IP = os.getenv("ESP_IP")
ESP_KEY = os.getenv("ESP_API_KEY")
JSON_FILE = os.path.join(folder_path, "jadwal.json")

# SET TIMEZONE BALI
MY_TZ = pytz.timezone('Asia/Makassar') 

# --- LOGGING ---
logging.basicConfig(format='%(asctime)s - %(message)s', level=logging.INFO)
logging.getLogger("httpx").setLevel(logging.WARNING)
logging.getLogger("telegram").setLevel(logging.WARNING)

user_input_mode = {} 

# ================= DATABASE =================
def baca_json():
    if not os.path.exists(JSON_FILE): return []
    try:
        with open(JSON_FILE, 'r') as f: return json.load(f)
    except: return []

def tulis_json(data):
    try:
        with open(JSON_FILE, 'w') as f: json.dump(data, f, indent=4)
    except Exception as e: print(f"Err JSON: {e}")

# ================= HELPER STATUS =================
def get_lamp_status():
    """Mengambil status tanpa melakukan toggle"""
    try:
        url_status = f"{ESP_IP}/status?key={ESP_KEY}"
        r = requests.get(url_status, timeout=3)
        if r.status_code == 200:
            state = r.json().get('state') # True=Mati, False=Hidup
            return "MATI 🌑" if state else "HIDUP 💡"
        return "ERROR ⚠️"
    except:
        return "OFFLINE 🔌"

# ================= LOGIKA LAMPU =================
def paksa_lampu_nyala():
    try:
        url_status = f"{ESP_IP}/status?key={ESP_KEY}"
        r = requests.get(url_status, timeout=5)
        if r.status_code == 200:
            state = r.json().get('state') 
            if state == False:
                return True, "Sudah Menyala (Aman)"
            else:
                url_toggle = f"{ESP_IP}/toggle?key={ESP_KEY}"
                requests.get(url_toggle, timeout=5)
                return True, "Berhasil Dihidupkan"
        return False, "Gagal Cek Status"
    except Exception as e:
        return False, str(e)

# ================= ALARM CALLBACK =================
async def alarm_callback(context: ContextTypes.DEFAULT_TYPE):
    data = context.job.data
    chat_id = data.get('chat_id')
    waktu = data.get('waktu')
    tipe = data.get('tipe')
    job_id = data.get('id')

    sukses, info = paksa_lampu_nyala()
    status_lampu = "NYALA 💡" if sukses else "GAGAL ❌"

    try:
        msg = await context.bot.send_message(
            chat_id=chat_id,
            text=f"🔔 **REMINDER JAM {waktu}**\nStatus Lampu: **{status_lampu}**\nInfo: _{info}_",
            parse_mode='Markdown'
        )
    except: msg = None

    if tipe == 'Sekali':
        semua = baca_json()
        sisa = [j for j in semua if j['id'] != job_id]
        tulis_json(sisa)
    
    if msg:
        await asyncio.sleep(60) 
        try: await msg.delete()
        except: pass

# ================= MENU & HANDLERS =================
async def menu_utama(update: Update, context: ContextTypes.DEFAULT_TYPE):
    # Ambil status dulu sebelum nampilin menu
    status = get_lamp_status()
    keyboard = [[InlineKeyboardButton("💡 SAKLAR (TOGGLE)", callback_data='saklar')],
                [InlineKeyboardButton("⏰ ATUR JADWAL", callback_data='menu_jadwal')]]
    text = f"🤖 **PANEL KONTROL**\nStatus Lampu: **{status}**"
    
    if update.callback_query: await update.callback_query.message.edit_text(text, reply_markup=InlineKeyboardMarkup(keyboard), parse_mode='Markdown')
    else: await update.message.reply_text(text, reply_markup=InlineKeyboardMarkup(keyboard), parse_mode='Markdown')

async def handle_tombol(update: Update, context: ContextTypes.DEFAULT_TYPE):
    query = update.callback_query
    await query.answer()
    data = query.data

    if data == 'saklar':
        try:
            r = requests.get(f"{ESP_IP}/toggle?key={ESP_KEY}", timeout=3)
            state = r.json()['state']
            status = "MATI 🌑" if state else "HIDUP 💡"
            kb = [[InlineKeyboardButton("💡 SAKLAR (TOGGLE)", callback_data='saklar')],
                  [InlineKeyboardButton("⏰ ATUR JADWAL", callback_data='menu_jadwal')]]
            await query.message.edit_text(f"✅ Berhasil!\nStatus Lampu: **{status}**", reply_markup=InlineKeyboardMarkup(kb), parse_mode='Markdown')
        except: await query.message.edit_text("❌ ESP Offline")

    elif data == 'menu_jadwal':
        status = get_lamp_status()
        kb = [
            [InlineKeyboardButton("🔔 SEKALI", callback_data='set_sekali'), InlineKeyboardButton("🔁 RUTIN", callback_data='set_rutin')],
            [InlineKeyboardButton("📋 LIST", callback_data='list'), InlineKeyboardButton("🗑 HAPUS", callback_data='hapus')],
            [InlineKeyboardButton("🔙 KEMBALI", callback_data='main')]
        ]
        await query.message.edit_text(f"📅 **MENU JADWAL**\nStatus Lampu: **{status}**", reply_markup=InlineKeyboardMarkup(kb), parse_mode='Markdown')

    elif data == 'main': await menu_utama(update, context)

    elif data in ['set_sekali', 'set_rutin']:
        mode = 'Sekali' if data == 'set_sekali' else 'Rutin'
        user_input_mode[query.from_user.id] = {'mode': mode, 'msg_id': query.message.message_id}
        sent = await query.message.reply_text(f"📝 **INPUT {mode.upper()}**\nKetik jam: `20.00`", parse_mode='Markdown')
        user_input_mode[query.from_user.id]['bot_msg_id'] = sent.message_id

    elif data == 'list':
        d = baca_json()
        msg = "📋 **LIST JADWAL:**\n" + ("\n".join([f"{i+1}. {j['waktu']} ({j['tipe']})" for i,j in enumerate(d)]) if d else "📭 Kosong.")
        await query.message.edit_text(msg, reply_markup=InlineKeyboardMarkup([[InlineKeyboardButton("🔙 KEMBALI", callback_data='menu_jadwal')]]), parse_mode='Markdown')

    elif data == 'hapus':
        d = baca_json()
        if not d:
            await query.message.edit_text("📭 Kosong.", reply_markup=InlineKeyboardMarkup([[InlineKeyboardButton("🔙 KEMBALI", callback_data='menu_jadwal')]]))
            return
        msg_list = "\n".join([f"**{i+1}.** {j['waktu']} ({j['tipe']})" for i,j in enumerate(d)])
        text_hapus = f"🗑 **HAPUS JADWAL**\n\n{msg_list}\n\n👇 **Ketik Nomor:**"
        user_input_mode[query.from_user.id] = {'mode': 'Hapus', 'msg_id': query.message.message_id}
        sent = await query.message.reply_text(text_hapus, parse_mode='Markdown')
        user_input_mode[query.from_user.id]['bot_msg_id'] = sent.message_id

async def handle_pesan(update: Update, context: ContextTypes.DEFAULT_TYPE):
    uid = update.effective_user.id
    if uid not in user_input_mode: return
    text = update.message.text
    state = user_input_mode[uid]
    try: 
        await update.message.delete()
        await context.bot.delete_message(update.effective_chat.id, state['bot_msg_id'])
    except: pass
    del user_input_mode[uid]

    if state['mode'] == 'Hapus':
        d = baca_json()
        try:
            idx = int(text) - 1
            target = d.pop(idx)
            tulis_json(d)
            jobs = context.job_queue.get_jobs_by_name(target['id'])
            for j in jobs: j.schedule_removal()
            await context.bot.edit_message_text(chat_id=update.effective_chat.id, message_id=state['msg_id'], text=f"✅ Dihapus: **{target['waktu']}**", reply_markup=InlineKeyboardMarkup([[InlineKeyboardButton("🔙", callback_data='menu_jadwal')]]), parse_mode='Markdown')
        except: pass
        return

    try:
        jam, menit = map(int, text.replace('.', ':').split(':'))
        waktu_str = f"{jam:02d}.{menit:02d}"
        job_id = f"id_{int(datetime.now().timestamp())}"
        data_dict = {'id': job_id, 'chat_id': update.effective_chat.id, 'waktu': waktu_str, 'tipe': state['mode']}
        
        if state['mode'] == 'Sekali':
            now = datetime.now(MY_TZ)
            target = now.replace(hour=jam, minute=menit, second=0)
            if target < now: target += timedelta(days=1)
            context.job_queue.run_once(alarm_callback, target, data=data_dict, name=job_id)
        else:
            context.job_queue.run_daily(alarm_callback, time(jam, menit, tzinfo=MY_TZ), data=data_dict, name=job_id)

        d = baca_json()
        d.append(data_dict)
        tulis_json(d)
        await context.bot.edit_message_text(chat_id=update.effective_chat.id, message_id=state['msg_id'], text=f"✅ Diset: **{waktu_str}**", reply_markup=InlineKeyboardMarkup([[InlineKeyboardButton("🔙 KEMBALI", callback_data='menu_jadwal')]]), parse_mode='Markdown')
    except Exception as e: print(e)

async def post_init(app: Application):
    d = baca_json()
    for j in d:
        try:
            jam, menit = map(int, j['waktu'].replace('.', ':').split(':'))
            if j['tipe'] == 'Rutin': app.job_queue.run_daily(alarm_callback, time(jam, menit, tzinfo=MY_TZ), data=j, name=j['id'])
            elif j['tipe'] == 'Sekali':
                now = datetime.now(MY_TZ)
                target = now.replace(hour=jam, minute=menit, second=0)
                if target < now: target += timedelta(days=1)
                app.job_queue.run_once(alarm_callback, target, data=j, name=j['id'])
        except: pass

if __name__ == '__main__':
    if not os.path.exists(JSON_FILE): tulis_json([])
    app = ApplicationBuilder().token(TOKEN).post_init(post_init).build()
    app.add_handler(CommandHandler("start", menu_utama))
    app.add_handler(CallbackQueryHandler(handle_tombol))
    app.add_handler(MessageHandler(filters.TEXT & (~filters.COMMAND), handle_pesan))
    print("🤖 BOT FULL STATUS - START!")
    app.run_polling()
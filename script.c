const SHEET_ID = "1K_kwyrp8-KwjT2tNOvybW0TPLEZAqLa_cHFyubIU5rY";
const EMAIL_TO = ["mauriciomts99@gmail.com","marioeduardovilasboas@gmail.com"];
const TELEGRAM_TOKEN = "7551922735:AAHEiK5hMv81a1g3WJsDXY-EKLZ7QQ13FKM";
const TELEGRAM_CHAT_ID = "5354270923";
const DRIVE_FOLDER_ID = "1vg_lG4J11vRUMkwEf_UwCYn9UaNOJdFL";

function doPost(e) {
  try {
    const data = JSON.parse(e.postData.contents);
    const now = new Date();
    const formattedDate = Utilities.formatDate(
      now,
      Session.getScriptTimeZone(),
      "dd/MM/yyyy HH:mm:ss"
    );
    let imageUrl = "";

    // 1) Lida com a imagem
    if (data.imagemBase64 && DRIVE_FOLDER_ID) {
      const decodedImage = Utilities.base64Decode(
        data.imagemBase64,
        Utilities.Charset.UTF_8
      );
      const blob = Utilities.newBlob(
        decodedImage,
        data.imagemMimeType,
        `img_${Date.now()}_${data.imagemNome}`
      );
      const folder = DriveApp.getFolderById(DRIVE_FOLDER_ID);
      const file = folder.createFile(blob);
      file.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);
      imageUrl = file.getUrl();
    }

    // 2) Salva na planilha
    const ss = SpreadsheetApp.openById(SHEET_ID);
    const sheet =
      ss.getSheetByName("Respostas") || ss.insertSheet("Respostas", 0);
    if (sheet.getLastRow() === 0) {
      sheet.appendRow([
        "Data e Hora",
        "Nome",
        "E-mail",
        "Telefone",
        "Endereço",
        "Categoria",
        "Descrição",
        "Link da Imagem",
      ]);
    }
    sheet.appendRow([
      formattedDate,
      data.nome,
      data.email || "",
      data.telefone || "",
      data.endereco || "",
      data.categoria || "",
      data.descricao || "",
      imageUrl,
    ]);

    // 3) Envia para o Telegram
    if (TELEGRAM_TOKEN && TELEGRAM_TOKEN !== "COLE_AQUI_O_TOKEN_DO_SEU_BOT") {
      let message =
        `<b>📰 Radar Cidadão - Novo Registro</b>\n\n` +
        `<b>📅 Data:</b> ${formattedDate}\n` +
        `<b>👤 Nome:</b> ${data.nome}\n` +
        `<b>📍 Endereço:</b> ${data.endereco || "Não informado"}\n` +
        `<b>🏷️ Categoria:</b> ${data.categoria}\n\n` +
        `<b>📝 Descrição:</b>\n${data.descricao || ""}` +
        (imageUrl ? `\n\n📷 <a href="${imageUrl}">Abrir imagem</a>` : "");

      UrlFetchApp.fetch(
        `https://api.telegram.org/bot${TELEGRAM_TOKEN}/sendMessage`,
        {
          method: "post",
          contentType: "application/json",
          payload: JSON.stringify({
            chat_id: TELEGRAM_CHAT_ID,
            text: message,
            parse_mode: "HTML",
          }),
        }
      );
    }

    // 4) Envia e-mail formatado como notícia com logotipo
    if (EMAIL_TO.length > 0 && EMAIL_TO[0] !== "") {
      const subject = `[Radar Cidadão] Novo registro: ${data.categoria} - ${data.nome}`;
      let body = `
      <div style="font-family: Arial, sans-serif; max-width: 600px; margin: auto; border: 1px solid #ddd; border-radius: 8px; overflow: hidden;">
        <div style="background-color: #1c0097; color: white; padding: 16px; text-align: center; position: relative;">
          <img src="https://i.imgur.com/EcR4pHX.gif" alt="Logo" style="height: 50px; position: absolute; left: 16px; top: 16px;" />
          <h2 style="margin: 0;">Radar Cidadão</h2>
          <p style="margin: 0; font-size: 14px;">Novo registro recebido</p>
        </div>
        <div style="padding: 16px;">
          <p><b>📅 Data:</b> ${formattedDate}</p>
          <p><b>👤 Nome:</b> ${data.nome}</p>
          <p><b>📍 Endereço:</b> ${data.endereco || "Não informado"}</p>
          <p><b>🏷️ Categoria:</b> ${data.categoria}</p>
          <hr style="margin: 20px 0; border: none; border-top: 1px solid #eee;">
          <h3 style="margin-bottom: 8px;">📝 Descrição</h3>
          <p style="line-height: 1.5;">${(data.descricao || "").replace(
            /\n/g,
            "<br>"
          )}</p>
          ${
            imageUrl
              ? `<div style="margin-top: 20px; text-align: center;">
              <a href="${imageUrl}" target="_blank">
                <img src="${imageUrl}" alt="Imagem enviada" style="max-width: 100%; border-radius: 6px; box-shadow: 0 2px 8px rgba(0,0,0,0.1);" />
              </a>
            </div>`
              : ""
          }
        </div>
        <div style="background-color: #f4f4f4; padding: 10px; text-align: center; font-size: 12px; color: #666;">
          <p style="margin: 0;">Sistema automático • Radar Cidadão</p>
        </div>
      </div>
      `;

      MailApp.sendEmail({
        to: EMAIL_TO.join(","),
        subject: subject,
        htmlBody: body,
      });
    }

    return ContentService.createTextOutput(
      JSON.stringify({ status: "success" })
    ).setMimeType(ContentService.MimeType.JSON);
  } catch (err) {
    Logger.log(err);
    return ContentService.createTextOutput(
      JSON.stringify({ status: "error", message: err.message })
    ).setMimeType(ContentService.MimeType.JSON);
  }
}


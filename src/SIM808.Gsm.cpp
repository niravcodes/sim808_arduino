#include "SIM808.h"

AT_COMMAND(SEND_SMS, "+CMGS=\"%s\"");
AT_COMMAND(SEND_USSD, "+CUSD=1,\"*400#\",15"); // TODO


TOKEN_TEXT(CPIN, "+CPIN");
TOKEN_TEXT(CSQ, "+CSQ");
TOKEN_TEXT(CMGS, "+CMGS");
TOKEN_TEXT(CUSD, "+CUSD");

bool SIM808::simUnlock(const char* pin)
{
	sendAT(TO_F(TOKEN_CPIN), TO_F(TOKEN_WRITE), pin);

	return waitResponse(5000L) == 0;
}

size_t SIM808::getSimState(char *state, size_t stateSize)
{
	sendAT(TO_F(TOKEN_CPIN), TO_F(TOKEN_READ));
	if(waitResponse(5000L, TO_F(TOKEN_CPIN)) != 0) return 0;

	copyCurrentLine(state, stateSize, strlen_P(TOKEN_CPIN) + 2);

	return waitResponse() == 0 ?
		strlen(state) :
		0;
}

size_t SIM808::getImei(char *imei, size_t imeiSize)
{
	//AT+GSN does not have a response prefix, so we need to flush input
	//before sending the command
	flushInput();

	sendAT(S_F("+GSN"));	
	waitResponse(SIMCOMAT_DEFAULT_TIMEOUT, NULL); //consuming an extra line before the response. Undocumented

	if(waitResponse(SIMCOMAT_DEFAULT_TIMEOUT, NULL) != 0) return 0;
	copyCurrentLine(imei, imeiSize);

	return waitResponse() == 0?
		strlen(imei) :
		0;
}

SIM808SignalQualityReport SIM808::getSignalQuality()
{
	uint8_t quality;
	uint8_t errorRate;

	SIM808SignalQualityReport report = {99, 99, 1};

	sendAT(TO_F(TOKEN_CSQ));
	if(waitResponse(TO_F(TOKEN_CSQ)) != 0 ||
		!parseReply(',', (uint8_t)SIM808_SIGNAL_QUALITY_RESPONSE::SIGNAL_STRENGTH, &quality) ||
		!parseReply(',', (uint8_t)SIM808_SIGNAL_QUALITY_RESPONSE::BIT_ERROR_RATE, &errorRate) ||
		waitResponse())
		return report;

	report.rssi = quality;
	report.ber = errorRate;

	if (quality == 0) report.attenuation = -115;
	else if (quality == 1) report.attenuation = -111;
	else if (quality == 31) report.attenuation = -52;
	else if (quality > 31) report.attenuation = 1;
	else report.attenuation = map(quality, 2, 30, -110, -54);

	return report;
}

bool SIM808::setSmsMessageFormat(SIM808_SMS_MESSAGE_FORMAT format)
{
	sendAT(S_F("+CMGF="), (uint8_t)format);
	return waitResponse() == 0;
}

bool SIM808::sendSms(const char *addr, const char *msg)
{
	if (!setSmsMessageFormat(SIM808_SMS_MESSAGE_FORMAT::TEXT)) return false;
	sendFormatAT(TO_F(AT_COMMAND_SEND_SMS), addr);

	if (!waitResponse(S_F(">")) == 0) return false;

	SENDARROW;
	print(msg);
	print((char)0x1A);

	return waitResponse(60000L, TO_F(TOKEN_CMGS)) == 0 &&
		waitResponse() == 0;
}

bool SIM808::sendUssd(const char *pin, char* response, size_t responseSize)
{
	sendAT(TO_F(AT_COMMAND_SEND_USSD));

	if(waitResponse() != 0)
		return false;
	if (waitResponse(60000L, TO_F(TOKEN_CUSD)) == 0){
		copyCurrentLine(response, responseSize, strlen_P(TOKEN_CUSD) + 2);
		return true;
	}
	return false;
}


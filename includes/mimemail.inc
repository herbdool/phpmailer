<?php
// $Id$

include_once drupal_get_path('module', 'phpmailer') .'/phpmailer/class.phpmailer.php';

class MimeMailPHPMailer extends PHPMailer {
  /**
   * Constructor.
   */
  function MimeMailPHPMailer() {

    $this->IsSMTP();

    // Assemble host name(s).
    $protocol = variable_get('smtp_protocol', '');
    $this->Host = $protocol . variable_get('smtp_host', '');
    if ($backup = variable_get('smtp_hostbackup', '')) {
       $this->Host .= ';'. $protocol . $backup;
    }
    $this->Port = variable_get('smtp_port', '25');

    // Use SMTP authentication if both username and password are given.
    $this->Username = variable_get('smtp_username', '');
    $this->Password = variable_get('smtp_password', '');
    $this->SMTPAuth = ($this->Username != '' && $this->Password != '');

    // Set default From name.
    $from_name = variable_get('smtp_fromname', '');
    if ($from_name == '') {
      // Fall back on site name.
      $from_name = variable_get('site_name', 'Drupal');
    }
    $this->FromName = $from_name;

    $this->SMTPKeepAlive = variable_get('smtp_keepalive', 0);
    $this->SMTPDebug     = variable_get('smtp_debug', 0);
    $this->CharSet       = variable_get('smtp_charset', 'utf-8');

    // Adjust path to SMTP class.
    $this->PluginDir = drupal_get_path('module', 'phpmailer') .'/phpmailer/';
  }

  /**
   * Sets the language for all class error messages.
   */
  function SetLanguage($lang_type, $lang_path = null) {
    $PHPMAILER_LANG = array();

    $PHPMAILER_LANG["provide_address"] = t('You must provide at least one recipient email address.');
    $PHPMAILER_LANG["mailer_not_supported"] = t(' mailer is not supported.');
    $PHPMAILER_LANG["execute"] = t('Could not execute: ');
    $PHPMAILER_LANG["instantiate"] = t('Could not instantiate mail function.');
    $PHPMAILER_LANG["authenticate"] = t('SMTP Error: Could not authenticate.');
    $PHPMAILER_LANG["from_failed"] = t('The following From address failed: ');
    $PHPMAILER_LANG["recipients_failed"] = t('SMTP Error: The following recipients failed: ');
    $PHPMAILER_LANG["data_not_accepted"] = t('SMTP Error: Data not accepted.');
    $PHPMAILER_LANG["connect_host"] = t('SMTP Error: Could not connect to SMTP host.');
    $PHPMAILER_LANG["file_access"] = t('Could not access file: ');
    $PHPMAILER_LANG["file_open"] = t('File Error: Could not open file: ');
    $PHPMAILER_LANG["encoding"] = t('Unknown encoding: ');

    $this->language = $PHPMAILER_LANG;

    return true;
  }
}

/**
 * Sends out the email.
 *
 * @param $addresses
 *   Array of recipient addresses.
 * @param $subject
 *   Message subject.
 * @param $body
 *   Message body.
 * @param $from
 *   Sender e-mail address.
 * @param $header
 *   Message header.
 */
function mimemail_phpmailer_send($recipients, $subject, $body, $from, $header) {

  $mail = new MimeMailPHPMailer();

  settype($header, 'array');

  // Select which From address to use:
  // $from parameter takes precedence over $header['From'].
  if (isset($header['From'])) {
    // Check whether a full e-mail address has been specified
    // ("some name" <user@example.com>).
    if ($parsed = phpmailer_parse_address($header['From'])) {
      $mail->FromName = $parsed['name'];
      $from_address = $parsed['address'];
    }
    else {
      $from_address = $header['From'];
    }
    unset($header['From']);
  }
  if (empty($from)) {
    if (!empty($from_address)) {
      $from = $from_address;
    }
    else{
      $from = variable_get('site_mail', ini_get('sendmail_from'));
    }
  }
  else {
    if ($parsed = phpmailer_parse_address($from)) {
      $from = $parsed['address'];
    }
  }
  $mail->From = $from;

  // Set additional properties.
  $properties = array(
    'X-Priority' => 'Priority',
  );
  foreach ($properties as $source => $property) {
    if (isset($header[$source])) {
      $mail->$property = $header[$source];
      unset($header[$source]);
    }
  }

  // Always set by PHPMailer.
  unset($header['MIME-Version']);

  // Add remaining header lines.
  // Note: Any header lines MUST be checked by the caller for unwanted
  // newline characters to avoid header injection. Use PHPMailer::SecureHeader()
  // for that purpose.
  foreach ($header as $key => $value) {
    $mail->AddCustomHeader("$key:$value");
  }

  // Check if debugging is turned on and replace recipient.
  $debug_email = variable_get('phpmailer_debug_email', '');
  if ($debug_email) {
    $recipients = array($debug_email);
  }

  // Add recipients.
  foreach ($recipients as $recipient) {
    $mail->AddAddress($recipient);
  }

  $mail->Subject = $subject;

  // Create SMTP data header.
  $header = $mail->Createheader();

  if (!($result = $mail->SmtpSend($header, $body))) {
    watchdog('smtp', t('Error sending email: From: @from To: @to Error: @error', array('@from' => '<'. $from .'>', '@to' => '<'. implode('>, <', $recipients) .'>', '@error' => $mail->ErrorInfo)), WATCHDOG_ERROR);
  }
  // Commented out to benefit from keep-alive feature.
  //$mail->SmtpClose();

  return $result;
}


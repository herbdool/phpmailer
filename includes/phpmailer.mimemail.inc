<?php
// $Id$

/**
 * @file
 * Implements PHPMailer support on behalf of the Mime Mail module.
 */

module_load_include('inc', 'phpmailer', 'includes/phpmailer.class');

/**
 * Sends out the email.
 *
 * @param $message
 *   Message array structure.
 */
function mimemail_phpmailer_send($message) {
  static $mail;

  if (!isset($mail)) {
    $mail = new DrupalPHPMailer();
  }

  // Extract and assign e-mail addresses required for SMTP.
  // Display names are not required. Leave header intact.
  $address = phpmailer_parse_address($message['sender']);
  $mail->From = $address[0]['mail'];

  if (isset($message['headers']['CC'])) {
    foreach (phpmailer_parse_address($message['headers']['CC']) as $address) {
      $mail->AddCC($address['mail']);
    }
  }
  if (isset($message['headers']['BCC'])) {
    foreach (phpmailer_parse_address($message['headers']['BCC']) as $address) {
      $mail->AddBCC($address['mail']);
    }
  }

  // Replace recipient if set.
  if (variable_get('phpmailer_debug_email', '')) {
    $message['address'] = variable_get('phpmailer_debug_email', '');
  }

  // Set recipients.
  foreach (phpmailer_parse_address($message['address']) as $address) {
    $mail->AddAddress($address['mail']);
  }

  if ($mail->SMTPDebug) {
    ob_start();
  }

  $header = mimemail_rfc_headers($message['headers']);
  if (!($result = $mail->SmtpSend($header, $body))) {
    watchdog('phpmailer', $mail->ErrorInfo, NULL, WATCHDOG_ERROR);
  }

  // Reset object properties when keep-alive is enabled.
  if ($mail->SMTPKeepAlive) {
    $mail->Reset();
  }

  if ($mail->SMTPDebug) {
    if ($debug = ob_get_contents()) {
      drupal_set_message($debug);
    }
    ob_end_clean();
  }

  return $result;
}


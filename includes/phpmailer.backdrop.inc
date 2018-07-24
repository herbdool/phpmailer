<?php

/**
 * @file
 * Implements PHPMailer support on behalf of Backdrop core.
 */

/**
 * Sends an already formatted e-mail message composed by backdrop_mail().
 *
 * @param array $message
 *   A message array, as described in hook_mail_alter().
 *
 * @return bool
 *   TRUE if the mail was successfully accepted, otherwise FALSE.
 *
 * @see BackdropPHPMailer::mail()
 *
 * @todo Consider to either drop this entirely, or move into phpmailer.module
 *   as singleton to instantiate the BackdropPHPMailer class.
 */
function phpmailer_send(array $message) {
  $mail = &backdrop_static(__FUNCTION__);

  if (!isset($mail)) {
    $mail = new BackdropPHPMailer();
  }
  return $mail->mail($message);
}

<?php
// $Id$

/**
 * @file
 * Implements the base PHPMailer for Drupal class.
 */

module_load_include('php', 'phpmailer', 'phpmailer/class.phpmailer');

/**
 * Base PHPMailer for Drupal implementation with support for SMTP keep-alive
 * and setting a custom Return-Path.
 */
class DrupalPHPMailer extends PHPMailer {
  /**
   * Stores the Return-Path, which may be different from Sender.
   */
  public $ReturnPath = '';

  /**
   * Constructor.
   */
  public function __construct() {
    // Throw exceptions instead of echoing error messages (since 5.0.0).
    parent::__construct(TRUE);

    $this->IsSMTP();
    $this->Reset();

    $this->Host = variable_get('smtp_host', '');
    if ($backup = variable_get('smtp_hostbackup', '')) {
      $this->Host .= ';' . $backup;
    }
    $this->Port = variable_get('smtp_port', '25');
    $this->SMTPSecure = variable_get('smtp_protocol', '');

    // Use SMTP authentication if both username and password are given.
    $this->Username = variable_get('smtp_username', '');
    $this->Password = variable_get('smtp_password', '');
    $this->SMTPAuth = (bool)($this->Username != '' && $this->Password != '');

    $this->SMTPKeepAlive = variable_get('smtp_keepalive', 0);
    $this->SMTPDebug = variable_get('smtp_debug', 0);

    // Adjust path to SMTP class.
    $this->PluginDir = drupal_get_path('module', 'phpmailer') . '/phpmailer/';
  }

  /**
   * Wrapper around PHPMailer::Send() that properly handles exceptions.
   */
  public function Send() {
    try {
      $result = parent::Send();
    }
    catch (phpmailerException $e) {
      drupal_set_message($e->getMessage(), 'error');
      $result = FALSE;
    }
    return $result;
  }

  /**
   * (Re-)initialize properties after sending mail.
   */
  public function Reset() {
    $this->ClearAllRecipients();
    $this->ClearAttachments();
    $this->ClearCustomHeaders();

    $this->Priority    = 3;
    $this->CharSet     = variable_get('smtp_charset', 'utf-8');
    $this->ContentType = 'text/plain';
    $this->Encoding    = '8bit';

    // Set default From name.
    $from_name = variable_get('smtp_fromname', '');
    if ($from_name == '') {
      // Fall back on the site name.
      $from_name = variable_get('site_name', 'Drupal');
    }
    $this->FromName   = $from_name;
    $this->Sender     = '';
    $this->MessageID  = '';
    $this->ReturnPath = '';
  }

  /**
   * Destructor.
   */
  public function __destruct() {
    $this->SmtpClose();
  }

  /**
   * Public wrapper around PHPMailer::SmtpSend().
   */
  public function SmtpSend($header, $body) {
    $result = parent::SmtpSend($header, $body);
    // Reinitialize properties.
    $this->Reset();
    return $result;
  }

  /**
   * Set the language for all class error messages.
   *
   * To avoid having people to copy the languages/ directory, we provide
   * the english translations in this overridden method.
   * Drupal's language codes are different from those used by PHPMailer anyway,
   * and the messages will only ever appear in the watchdog reports.
   */
  public function SetLanguage($lang_type = 'en', $lang_path = 'language/') {
    $this->language = array(
      'provide_address' => 'You must provide at least one recipient email address.',
      'mailer_not_supported' => ' mailer is not supported.',
      'execute' => 'Could not execute: ',
      'instantiate' => 'Could not instantiate mail function.',
      'authenticate' => 'SMTP Error: Could not authenticate.',
      'from_failed' => 'The following From address failed: ',
      'recipients_failed' => 'SMTP Error: The following recipients failed: ',
      'data_not_accepted' => 'SMTP Error: Data not accepted.',
      'connect_host' => 'SMTP Error: Could not connect to SMTP host.',
      'file_access' => 'Could not access file: ',
      'file_open' => 'File Error: Could not open file: ',
      'encoding' => 'Unknown encoding: ',
      'signing' => 'Signing Error: ',
      'smtp_error' => 'SMTP server error: ',
      'empty_message' => 'Message body empty',
      'invalid_address' => 'Invalid address',
      'variable_set' => 'Cannot set or reset variable: '
    );
    return TRUE;
  }

  /**
   * Assemble the message header.
   *
   * PHPMailer always sets Return-Path to Sender, we want more flexibility.
   */
  public function CreateHeader() {
    $old_sender = $this->Sender;
    if ($this->ReturnPath != '') {
      $this->Sender = $this->ReturnPath;
    }
    $result = parent::CreateHeader();
    // Restore sender for use in MAIL FROM command.
    $this->Sender = $old_sender;
    return $result;
  }

  /**
   * Public wrapper around PHPMailer::RFCDate().
   */
  public static function RFCDate() {
    $tz = date('Z');
    $tzs = ($tz < 0) ? '-' : '+';
    $tz = abs($tz);
    $tz = (int)($tz / 3600) * 100 + ($tz % 3600) / 60;
    $result = sprintf("%s %s%04d", date('D, j M Y H:i:s'), $tzs, $tz);

    return $result;
  }
}


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
   * Stores the Return-Path, which can be different from Sender.
   */
  public $ReturnPath = '';

  /**
   * Constructor.
   */
  public function __construct() {
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
    $this->ErrorInfo   = '';

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
   * Sets the language for all class error messages.
   *
   * To avoid having people to copy the languages/ directory, we provide
   * the english translations in this overridden method.
   * Drupal's language code are different from those used by PHPMailer anyway,
   * and the messages only ever appear in the watchdog reports.
   */
  public function SetLanguage($lang_type = 'en', $lang_path = 'language/') {
    $PHPMAILER_LANG = array();
    $PHPMAILER_LANG['provide_address']      = 'You must provide at least one recipient email address.';
    $PHPMAILER_LANG['mailer_not_supported'] = ' mailer is not supported.';
    $PHPMAILER_LANG['execute']              = 'Could not execute: ';
    $PHPMAILER_LANG['instantiate']          = 'Could not instantiate mail function.';
    $PHPMAILER_LANG['authenticate']         = 'SMTP Error: Could not authenticate.';
    $PHPMAILER_LANG['from_failed']          = 'The following From address failed: ';
    $PHPMAILER_LANG['recipients_failed']    = 'SMTP Error: The following recipients failed: ';
    $PHPMAILER_LANG['data_not_accepted']    = 'SMTP Error: Data not accepted.';
    $PHPMAILER_LANG['connect_host']         = 'SMTP Error: Could not connect to SMTP host.';
    $PHPMAILER_LANG['file_access']          = 'Could not access file: ';
    $PHPMAILER_LANG['file_open']            = 'File Error: Could not open file: ';
    $PHPMAILER_LANG['encoding']             = 'Unknown encoding: ';
    $PHPMAILER_LANG['signing']              = 'Signing Error: ';
    $this->language = $PHPMAILER_LANG;
    return true;
  }

  /**
   * Assembles message header.
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
   * Returns the proper RFC 822 formatted date.
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


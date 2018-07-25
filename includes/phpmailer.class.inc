<?php

/**
 * @file
 * Implements the base PHPMailer for Backdrop class.
 */

/**
 * Class for implementing the PHPMailer library in Backdrop.
 *
 * Includes support for SMTP keep-alive and setting a custom Return-Path.
 */
class BackdropPHPMailer extends PHPMailer implements MailSystemInterface {

  /**
   * Stores the Return-Path, which may be different from Sender.
   *
   * @var string
   */
  public $ReturnPath = '';

  /**
   * Verbose debug output level configured for Backdrop.
   *
   * In order to be able to log error messages with actual error information and
   * see what actually went wrong for a particular message, PHPMailer::SMTPDebug
   * always needs to be enabled.
   *
   * BackdropPHPMailer::SmtpSend() overrides PHPMailer::SmtpSend() to capture the
   * debug output string and make it available for watchdog() calls.
   *
   * @var int
   *
   * @see PHPMailer::SMTPDebug
   * @see SMTP::do_debug
   * @see BackdropPHPMailer::SmtpSend()
   * @see BackdropPHPMailer::backdropDebugOutput
   */
  public $backdropDebug = 0;

  /**
   * Overrides PHPMailer::SMTPDebug to capture SMTP communication errors.
   *
   * @var int
   */
  public $SMTPDebug = 2;

  /**
   * Stores the verbose debug output of the SMTP communication.
   *
   * @var string
   */
  public $backdropDebugOutput = '';

  /**
   * Constructor.
   */
  public function __construct() {
    // Throw exceptions instead of dying (since 5.0.0).
    if (method_exists(get_parent_class($this), '__construct')) {
      parent::__construct(TRUE);
    }
    $config = config('phpmailer.settings');

    $this->IsSMTP();
    $this->Reset();

    $this->Host = $config->get('smtp_host');
    if ($backup = $config->get('smtp_hostbackup')) {
      $this->Host .= ';' . $backup;
    }
    $this->Port = $config->get('smtp_port');
    $this->SMTPSecure = $config->get('smtp_protocol');

    $this->SMTPOptions['ssl']['verify_peer'] = $config->get('ssl_verify_peer');
    $this->SMTPOptions['ssl']['verify_peer_name'] = $config->get('ssl_verify_peer_name');
    $this->SMTPOptions['ssl']['allow_self_signed'] = $config->get('ssl_allow_self_signed');

    // Use SMTP authentication if both username and password are given.
    $this->Username = $config->get('smtp_username');
    $this->Password = $config->get('smtp_password');
    $this->SMTPAuth = (bool) ($this->Username != '' && $this->Password != '');
    $auth_type = $config->get('smtp_auth_type');
    if ($this->SMTPAuth && $auth_type) {
      $this->AuthType = $auth_type;
    }

    $this->SMTPKeepAlive = $config->get('smtp_keepalive');

    $this->backdropDebug = $config->get('smtp_debug');
    if ($this->backdropDebug > $this->SMTPDebug && user_access('administer phpmailer settings')) {
      $this->SMTPDebug = $this->backdropDebug;
    }

    // Adjust path to SMTP class.
    $this->PluginDir = BACKDROP_ROOT . '/' . backdrop_get_path('module', 'phpmailer') . '/libraries/phpmailer/';
  }

  /**
   * Send mail via SMTP.
   *
   * Wrapper around PHPMailer::SmtpSend() with exception handling.
   */
  public function SmtpSend($header, $body) {
    if ($this->SMTPDebug) {
      // Clear possibly previously captured debug output.
      $this->backdropDebugOutput = '';
      ob_start();
    }

    try {
      $result = parent::SmtpSend($header, $body);

      // Close connection when not using SMTP keep-alive.
      if (!$this->SMTPKeepAlive) {
        $this->SmtpClose();
      }
    }
    catch (phpmailerException $exception) {
    }

    if ($this->SMTPDebug) {
      if ($this->backdropDebug && ($this->backdropDebugOutput = ob_get_contents())) {
        backdrop_set_message($this->backdropDebugOutput);
        if (config_get('phpmailer.settings', 'smtp_debug_log')) {
          watchdog('phpmailer', 'Output of communication with SMTP server:<br /><pre>!output</pre>', array('!output' => print_r($this->backdropDebugOutput, TRUE)), WATCHDOG_DEBUG);
        }
      }
      ob_end_clean();
    }

    // Reinitialize properties.
    $this->Reset();

    if (isset($exception)) {
      // Pass exception to caller.
      throw $exception;
    }
    return $result;
  }

  /**
   * Initialize or re-initialize properties after sending mail.
   */
  public function Reset() {
    $config = config('phpmailer.settings');

    $this->ClearAllRecipients();
    $this->ClearReplyTos();
    $this->ClearAttachments();
    $this->ClearCustomHeaders();

    $this->Priority    = 3;
    $this->CharSet     = $config->get('smtp_charset');
    $this->ContentType = 'text/plain';
    $this->Encoding    = '8bit';

    // Set default From name.
    $from_name = $config->get('smtp_fromname');
    if ($from_name == '') {
      // Fall back on the site name.
      $from_name = config_get('system.core', 'site_name');
    }
    $this->FromName   = $from_name;
    $this->Sender     = '';
    $this->MessageID  = '';
    $this->ReturnPath = '';
  }

  /**
   * Overrides PHPMailer::__destruct().
   */
  public function __destruct() {
    // Disable debug output if SMTP keep-alive is enabled.
    // PHP is most likely shutting down altogether (this class is instantiated
    // as a static singleton). Since logging facilities (e.g., database
    // connection) quite potentially have been shut down already, simply turn
    // off SMTP debugging. Without this override, debug output would be printed
    // on the screen and CLI output.
    if ($this->SMTPKeepAlive && isset($this->smtp->do_debug)) {
      $this->smtp->do_debug = 0;
    }
    parent::__destruct();
  }

  /**
   * Provide more user-friendly error messages.
   *
   * Note: messages should not end with a dot.
   */
  public function SetLanguage($langcode = 'en', $lang_path = 'language/') {
    // Retrieve English defaults to ensure all message keys are set.
    parent::SetLanguage('en');

    // Overload with Backdrop translations.
    $this->language = array(
      'authenticate'        => t('SMTP error: Could not authenticate.'),
      'connect_host'        => t('SMTP error: Could not connect to host.'),
      'data_not_accepted'   => t('SMTP error: Data not accepted.'),
      'smtp_connect_failed' => t('SMTP error: Could not connect to SMTP host.'),
      'smtp_error'          => t('SMTP server error:') . ' ',

      // Messages used during email generation.
      'empty_message'       => t('Message body empty'),
      'encoding'            => t('Unknown encoding:') . ' ',
      'variable_set'        => t('Cannot set or reset variable:') . ' ',

      'file_access'         => t('File error: Could not access file:') . ' ',
      'file_open'           => t('File error: Could not open file:') . ' ',

      // Non-administrative messages.
      'from_failed'         => t('The following From address failed:') . ' ',
      'invalid_address'     => t('Invalid address'),
      'provide_address'     => t('You must provide at least one recipient e-mail address.'),
      'recipients_failed'   => t('The following recipients failed:') . ' ',
    ) + $this->language;
    return TRUE;
  }

  /**
   * Public wrapper around PHPMailer::RFCDate().
   */
  public static function RFCDate() {
    $tz = date('Z');
    $tzs = ($tz < 0) ? '-' : '+';
    $tz = abs($tz);
    $tz = (int) ($tz / 3600) * 100 + ($tz % 3600) / 60;
    return sprintf("%s %s%04d", date('D, j M Y H:i:s'), $tzs, $tz);
  }

  /**
   * Concatenates and wraps the e-mail body for plain-text mails.
   *
   * Mails contain HTML by default. When using PHPMailer without MimeMail
   * module, plain-text e-mails are sent, which require the same processing as
   * in the DefaultMailSystem implementation.
   *
   * @param array $message
   *   A message array, as described in hook_mail_alter().
   *
   * @return array
   *   The formatted $message.
   *
   * @see DefaultMailSystem::format()
   */
  public function format(array $message) {
    // Join the body array into one string.
    $message['body'] = implode("\n\n", $message['body']);
    // Convert any HTML to plain-text.
    $message['body'] = backdrop_html_to_text($message['body']);
    // Wrap the mail body for sending.
    $message['body'] = backdrop_wrap_mail($message['body']);
    return $message;
  }

  /**
   * Sends an e-mail message composed by backdrop_mail().
   *
   * @param array $message
   *   A message array, as described in hook_mail_alter().
   *
   * @return bool
   *   TRUE if the mail was successfully accepted, otherwise FALSE.
   *
   * @see PHPMailer::Send()
   */
  public function mail(array $message) {
    $config = config('phpmailer.settings');
    $phpmailer_debug_email = settings_get('phpmailer_debug_email', '');
    
    try {
      // Parse 'From' e-mail address.
      $from = phpmailer_parse_address($message['from']);
      $from = reset($from);
      $this->From = $from['mail'];
      if ($from['name'] != '') {
        $this->FromName = $from['name'];
      }
      unset($message['headers']['From']);

      if ($phpmailer_debug_email === '') {
        // Set recipients.
        foreach (phpmailer_parse_address($message['to']) as $address) {
          $this->AddAddress($address['mail'], $address['name']);
        }
        // Extract CCs and BCCs from headers.
        if (!empty($message['headers']['Cc'])) {
          foreach (phpmailer_parse_address($message['headers']['Cc']) as $address) {
            $this->AddCC($address['mail'], $address['name']);
          }
        }
        if (!empty($message['headers']['Bcc'])) {
          foreach (phpmailer_parse_address($message['headers']['Bcc']) as $address) {
            $this->AddBCC($address['mail'], $address['name']);
          }
        }
      }
      else {
        // Reroute to debug e-mail address.
        $this->AddAddress($phpmailer_debug_email);
      }
      unset($message['headers']['Cc'], $message['headers']['Bcc']);

      // Extract Reply-To from headers.
      if (isset($message['headers']['Reply-To'])) {
        foreach (phpmailer_parse_address($message['headers']['Reply-To']) as $address) {
          $this->AddReplyTo($address['mail'], $address['name']);
        }
        unset($message['headers']['Reply-To']);
      }
      elseif ($config->get('smtp_always_replyto')) {
        // If no Reply-To header has been explicitly set, use the From address
        // to be able to respond to e-mails sent via Google Mail.
        $this->AddReplyTo($from['mail'], $from['name']);
      }

      // Extract Content-Type and charset.
      if (isset($message['headers']['Content-Type'])) {
        $content_type = explode(';', $message['headers']['Content-Type']);
        $this->ContentType = trim(array_shift($content_type));
        foreach ($content_type as $param) {
          $param = explode('=', $param, 2);
          $key = trim($param[0]);
          if ($key == 'charset') {
            $this->CharSet = trim($param[1]);
          }
          else {
            $this->ContentType .= '; ' . $key . '=' . trim($param[1]);
          }
        }
        unset($message['headers']['Content-Type']);
      }

      // Set additional properties.
      $properties = array(
        'X-Priority'                => 'Priority',
        'Content-Transfer-Encoding' => 'Encoding',
        'Sender'                    => 'Sender',
        'Message-ID'                => 'MessageID',
        'Return-Path'               => 'ReturnPath',
      );
      foreach ($properties as $source => $property) {
        if (isset($message['headers'][$source])) {
          $this->$property = $message['headers'][$source];
          unset($message['headers'][$source]);
        }
      }

      // This one is always set by PHPMailer.
      unset($message['headers']['MIME-Version']);

      // Add remaining header lines.
      // Note: Any header lines MUST already be checked by the caller for
      // unwanted newline characters to avoid header injection.
      // @see PHPMailer::SecureHeader()
      foreach ($message['headers'] as $key => $value) {
        $this->AddCustomHeader("$key:$value");
      }

      $this->Subject = $message['subject'];
      $this->Body = $message['body'];

      return $this->Send();
    }
    catch (phpmailerException $e) {
      // Log the error including verbose debug information.
      // Since DBLog module is the most common case, we use HTML to format the
      // message for visual inspection. For sites running with Syslog or other
      // logging modules, we put the actual values on separate lines (\n), so
      // the surrounding HTML markup doesn't get too disturbing.
      //
      // Message is a safe t() string from BackdropPHPMailer::SetLanguage().
      $output = $e->getMessage();
      // Attempt to delimit summary from full message.
      $output .= " \n";
      $arguments = array();
      // Append SMTP communication output.
      if ($this->backdropDebugOutput) {
        // PHPMailer debug output contains HTML linebreaks. PRE is more
        // readable.
        $this->backdropDebugOutput = str_replace('<br />', '', $this->backdropDebugOutput);
        $output .= '<p><strong>Server response:</strong></p>';
        $output .= "<pre>\n@smtp_output\n</pre>";
        $arguments += array(
          '@smtp_output' => $this->backdropDebugOutput,
        );
      }
      // We need to log the message in order to be able to debug why the server
      // responded with an error. The mail body may contain passwords and other
      // sensitive information, which should not be logged. Since all kind of
      // mails are processed and Backdrop provides no way to mark sensible data,
      // it is technically impossible prevent logging in all cases.
      // Remove $params; they've already been processed and may contain sensible
      // data.
      unset($message['params']);

      // Subject.
      $output .= "<p><strong>Subject:</strong> \n@subject\n</p>";
      $arguments += array(
        '@subject' => $message['subject'],
      );
      unset($message['subject']);
      // Body.
      $output .= '<p><strong>Body:</strong></p>';
      $output .= "<pre>\n@body\n</pre>";
      $arguments += array(
        '@body' => $message['body'],
      );
      unset($message['body']);
      // Rest of $message.
      $output .= '<p><strong>Message:</strong></p>';
      $output .= "<pre>\n@message\n</pre>";
      $arguments += array(
        '@message' => var_export($message, TRUE),
      );
      watchdog('phpmailer', $output, $arguments, WATCHDOG_ERROR);
      return FALSE;
    }
  }

}

<?php

/**
 * @file
 * Administrative functions for PHPMailer integration module.
 */

/**
 * Form builder for both the Mime Mail settings and our own settings page.
 *
 * @ingroup forms
 * @see phpmailer_settings_form_validate()
 * @see phpmailer_settings_form_submit()
 */
function phpmailer_settings_form($form, $form_state) {
  $config_phpmailer = config('phpmailer.settings');
  $config_mimemail = config('mimemail.settings');
  $form['#config'] = 'phpmailer.settings';
  $form['smtp_on'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use PHPMailer to send e-mails'),
    '#default_value' => $config_phpmailer->get('smtp_on'),
    '#description' => t('When enabled, PHPMailer will be used to deliver all site e-mails.'),
  );
  // Only allow to send all e-mails if Mime Mail is not configured the same
  // (mimemail_alter is the counterpart to smtp_on).
  if (module_exists('mimemail') && $config_mimemail->get('mimemail_alter')) {
    $form['smtp_on']['#disabled'] = TRUE;
    $form['smtp_on']['#default_value'] = 0;
    $form['smtp_on']['#description'] = t('MimeMail has been detected. To enable PHPMailer for mail transport, go to the <a href="@url">MimeMail settings page</a> and select PHPMailer from the available e-mail engines.', array('@url' => url('admin/config/system/mimemail')));
  }
  elseif (!$config_phpmailer->get('smtp_on') && empty($form_state['values'])) {
    backdrop_set_message(t('PHPMailer is currently disabled.'), 'warning');
  }

  $form['server']['smtp_host'] = array(
    '#type' => 'textfield',
    '#title' => t('Primary SMTP server'),
    '#default_value' => $config_phpmailer->get('smtp_host'),
    '#description' => t('The host name or IP address of your primary SMTP server.  Use !gmail-smtp for Google Mail.', array('!gmail-smtp' => '<code>smtp.gmail.com</code>')),
    '#required' => TRUE,
  );
  $form['server']['smtp_hostbackup'] = array(
    '#type' => 'textfield',
    '#title' => t('Backup SMTP server'),
    '#default_value' => $config_phpmailer->get('smtp_hostbackup'),
    '#description' => t('Optional host name or IP address of a backup server, if the primary server fails.  You may override the default port below by appending it to the host name separated by a colon.  Example: !hostname', array('!hostname' => '<code>localhost:465</code>')),
  );
  $form['server']['smtp_port'] = array(
    '#type' => 'textfield',
    '#title' => t('SMTP port'),
    '#size' => 5,
    '#maxlength' => 5,
    '#default_value' => $config_phpmailer->get('smtp_port'),
    '#description' => t('The standard SMTP port is 25, for Google Mail use 465.'),
    '#required' => TRUE,
  );
  $form['server']['smtp_protocol'] = array(
    '#type' => 'select',
    '#title' => t('Use secure protocol'),
    '#default_value' => $config_phpmailer->get('smtp_protocol'),
    '#options' => array('' => t('No'), 'ssl' => t('SSL'), 'tls' => t('TLS')),
    '#description' => t('Whether to use an encrypted connection to communicate with the SMTP server.  Google Mail requires SSL.'),
  );
  if (!function_exists('openssl_open')) {
    $form['server']['smtp_protocol']['#default_value'] = '';
    $form['server']['smtp_protocol']['#disabled'] = TRUE;
    $form['server']['smtp_protocol']['#description'] .= ' ' . t('Note: This option has been disabled since your PHP installation does not seem to have support for OpenSSL.');
    config_set('phpmailer.settings', 'smtp_protocol', '');
  }

  $form['auth'] = array(
    '#type' => 'fieldset',
    '#title' => t('SMTP authentication'),
    '#description' => t('Leave blank if your SMTP server does not require authentication.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $auth_options = array(
    '' => '',
    'CRAM-MD5' => 'CRAM-MD5',
    'LOGIN' => 'LOGIN',
    'PLAIN' => 'PLAIN',
    'NTLM' => 'NTLM',
    'XOAUTH2' => 'XOAUTH2',
  );
  $description = t('If you do not specify an authentication type for use when authenticating with the server, the PHPMailer library will attempt to guess one based on the infomration it receives from the server.');
  $description .= ' ' . t('If it does not work and you receive notices about failed authentication, try specifying the authentication type.');
  $description .= '<br />' . t('If you are unsure which type to use, consider the following guidelines:');
  $description .= '<ul>';
  $description .= '<li>' . t('If the connection is NOT using a secure protocol, try CRAM-MD5 first, then try LOGIN or PLAIN.') . '</li>';
  $description .= '<li>' . t('If the connection is using a secure protocol, you can try LOGIN or PLAIN first, then CRAM-MD5.') . '</li>';
  $description .= '</ul>';
  $description .= t('(CRAM-MD5 hashes the password before sending it, so it is not transmitted "in the clear.")');
  $description .= '<br />' . t('If none of those options work, contact your email hoster to find out what their SMTP server authentication type is.');
  $description .= '<br />' . t('The PHPMailer library only supports those types listed.');
  $form['auth']['smtp_auth_type'] = array(
    '#type' => 'select',
    '#title' => t('Authentication type'),
    '#default_value' => $config_phpmailer->get('smtp_auth_type'),
    '#options' => $auth_options,
    '#description' => $description,
  );
  $form['auth']['smtp_username'] = array(
    '#type' => 'textfield',
    '#title' => t('Username'),
    '#default_value' => $config_phpmailer->get('smtp_username'),
    '#description' => t('For Google Mail, enter your username including "@gmail.com".'),
  );
  if (!$config_phpmailer->get('smtp_hide_password')) {
    $form['auth']['smtp_password'] = array(
      '#type' => 'textfield',
      '#title' => t('Password'),
      '#default_value' => $config_phpmailer->get('smtp_password'),
    );
    $form['auth']['smtp_hide_password'] = array(
      '#type' => 'checkbox',
      '#title' => t('Hide password'),
      '#default_value' => 0,
      '#description' => t("Check this option to permanently hide the plaintext password from peeking eyes. You may still change the password afterwards, but it won't be displayed anymore."),
    );
  }
  else {
    $have_password = ($config_phpmailer->get('smtp_password') != '');
    $form['auth']['smtp_password'] = array(
      '#type' => 'password',
      '#title' => $have_password ? t('Change password') : t('Password'),
      '#description' => $have_password ? t('Leave empty if you do not intend to change the current password.') : '',
    );
  }

  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced SMTP settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['advanced']['smtp_fromname'] = array(
    '#type' => 'textfield',
    '#title' => t('"From" name'),
    '#default_value' => $config_phpmailer->get('smtp_fromname'),
    '#description' => t('Enter a name that should appear as the sender for all messages.  If left blank the site name will be used instead: %sitename.', array('%sitename' => config_get('system.core', 'site_name'))),
  );
  $form['advanced']['smtp_always_replyto'] = array(
    '#type' => 'checkbox',
    '#title' => t('Always set "Reply-To" address'),
    '#default_value' => $config_phpmailer->get('smtp_always_replyto'),
    '#description' => t('Enables setting the "Reply-To" address to the original sender of the message, if unset.  This is required when using Google Mail, which would otherwise overwrite the original sender.'),
  );
  $form['advanced']['smtp_keepalive'] = array(
    '#type' => 'checkbox',
    '#title' => t('Keep connection alive'),
    '#default_value' => $config_phpmailer->get('smtp_keepalive'),
    '#description' => t('Whether to reuse an existing connection during a request.  Improves performance when sending a lot of e-mails at once.'),
  );
  $form['advanced']['smtp_encode_headers'] = array(
    '#type' => 'checkbox',
    '#title' => t('Encode headers'),
    '#default_value' => $config_phpmailer->get('smtp_encode_headers'),
    '#description' => t('If you have subjects and names with accented, etc. characters, check this box to UTF-8 encode them before sending the message.'),
  );
  $debug_options = array(
    0 => t('Disabled'),
    1 => t('Errors only'),
    2 => t('Server responses'),
    4 => t('Full communication'),
  );
  $form['advanced']['smtp_debug'] = array(
    '#type' => 'select',
    '#title' => t('Debug level'),
    '#default_value' => $config_phpmailer->get('smtp_debug'),
    '#options' => $debug_options,
    '#description' => t("Debug the communication with the SMTP server. You normally shouldn't enable this unless you're trying to debug e-mail sending problems."),
  );
  $form['advanced']['smtp_debug_log'] = array(
    '#type' => 'checkbox',
    '#title' => t("Record debugging output in Backdrop's log"),
    '#default_value' => $config_phpmailer->get('smtp_debug_log'),
    '#description' => t("If this is checked, the debugging out put that is produced will also be recorded in Backdrop's database log."),
    '#states' => array(
      'visible' => array(
        ':input[name=smtp_debug]' => array('!value' => 0),
      ),
    ),
  );

  $form['advanced']['ssl_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced SSL settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('If you are attempting to connect to a broken or malconfigured secure mail server, you can try toggling one or more of these options. <strong>If changing any of these options allows connection to the server, you should consider either fixing the SSL certifcate, or using a different SMTP server, as the connection may be insecure.</strong>'),
  );
  $form['advanced']['ssl_settings']['ssl_verify_peer'] = array(
    '#type' => 'checkbox',
    '#title' => t('Verfy peer'),
    '#default_value' => $config_phpmailer->get('ssl_verify_peer'),
    '#description' => t('If this is checked, it will require verification of the SSL certificate being used on the mail server.'),
  );
  $form['advanced']['ssl_settings']['ssl_verify_peer_name'] = array(
    '#type' => 'checkbox',
    '#title' => t('Verfy peer name'),
    '#default_value' => $config_phpmailer->get('ssl_verify_peer_name'),
    '#description' => t("If this is checked, it will require verification of the mail server's name in the SSL certificate."),
  );
  $form['advanced']['ssl_settings']['ssl_allow_self_signed'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow self signed'),
    '#default_value' => $config_phpmailer->get('ssl_allow_self_signed'),
    '#description' => t('If this is checked, it will allow conecting to a mail server with a self-signed SSL certificate. (This requires "Verfy peer" to be enabled.)'),
    '#states' => array(
      'enabled' => array(
        ':input[name="ssl_verify_peer"]' => array('checked' => TRUE),
      ),
    ),
  );

  // Send a test email message if an address has been entered.
  if ($test_address = $config_phpmailer->get('phpmailer_test')) {
    // Clear first to avoid unintended resending in case of an error.
    $config_phpmailer->set('phpmailer_test', '');
    $config_phpmailer->save();
    // If PHPMailer is enabled, send via regular backdrop_mail().
    if (phpmailer_enabled()) {
      $result = backdrop_mail('phpmailer', 'test', $test_address, NULL);
    }
    // Otherwise, prepare and send the test mail manually.
    else {
      // Prepare the message without sending.
      $message = backdrop_mail('phpmailer', 'test', $test_address, NULL, array(), NULL, FALSE);
      // Send the message. backdrop_mail_wrapper() is only defined when PHPMailer
      // is enabled, so backdrop_mail_send() cannot be used.
      // @see backdrop_mail_send()
      module_load_include('inc', 'phpmailer', 'includes/phpmailer.backdrop');
      $result = phpmailer_send($message);
    }

    $replacements = array(
      '%email' => $test_address,
      '!watchdog-url' => l(t('check the logs'), 'admin/reports/dblog'),
    );
    $notice = t('A test e-mail has been sent to %email. You might still want to !watchdog-url for possible error messages.', $replacements);
    $message_type = 'status';
    if (empty($result) || (is_array($result) && empty($result['result']))) {
      $notice = t('A test e-mail has NOT been sent to %email. You should !watchdog-url for error messages.', $replacements);
      $message_type = 'error';
    }
    backdrop_set_message($notice, $message_type);
  }

  $form['test'] = array(
    '#type' => 'fieldset',
    '#title' => t('Test configuration'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['test']['phpmailer_test'] = array(
    '#type' => 'textfield',
    '#title' => t('Recipient'),
    '#default_value' => '',
    '#description' => t('Type in an address to have a test e-mail sent there.'),
  );

  $form['#submit'] = array('phpmailer_settings_form_submit');

  return system_settings_form($form);
}

/**
 * Form validation function.
 *
 * @see phpmailer_settings_form()
 */
function phpmailer_settings_form_validate($form, &$form_state) {
  if ($form_state['values']['smtp_on']) {
    if (intval($form_state['values']['smtp_port']) == 0) {
      form_set_error('smtp_port', t('You must enter a valid SMTP port number.'));
    }
  }
}

/**
 * Form submit function.
 *
 * @see phpmailer_settings_form()
 */
function phpmailer_settings_form_submit($form, &$form_state) {
  $config_mail = config('system.mail');
  $config_phpmailer = config('phpmailer.settings');
  // Enable/disable mail sending subsystem.
  if ($form_state['values']['smtp_on']) {
    if (!phpmailer_enabled()) {
      $config_mail->set('default-system', 'BackdropPHPMailer');
      $config_mail->save();

      backdrop_set_message(t('PHPMailer will be used to deliver all site e-mails.'));
      watchdog('phpmailer', 'PHPMailer has been enabled.');
    }
  }
  elseif (phpmailer_enabled()) {
    // Remove PHPMailer from all mail keys it is configured for.
    $mail_system = $config_mail->get();
    foreach ($mail_system as $key => $class) {
      if ($class == 'BackdropPHPMailer') {
        $config_mail->set($key, 'DefaultMailSystem');
      }
    }
    $config_mail->save();

    backdrop_set_message(t('PHPMailer has been disabled.'));
    watchdog('phpmailer', 'PHPMailer has been disabled.');
  }

  // Log configuration changes.
  $settings = array('host', 'port', 'protocol', 'username');
  // Ignore empty passwords if hide password is active.
  if ($config_phpmailer->get('smtp_hide_password') && $form_state['values']['smtp_password'] == '') {
    unset($form_state['values']['smtp_password']);
  }
  else {
    $settings[] = 'password';
  }
  foreach ($settings as $setting) {
    if ($form_state['values']['smtp_' . $setting] != $config_phpmailer->get('smtp_' . $setting)) {
      watchdog('phpmailer', 'SMTP configuration changed.');
      break;
    }
  }
}

/**
 * Implements hook_mail().
 */
function phpmailer_mail($key, &$message, $params) {
  $message['subject'] = t('PHPMailer test e-mail');
  $message['body'][] = t('Your site is properly configured to send e-mails using the <strong>PHPMailer</strong> library.');
}

/**
 * Renders an HTML mail preview in the browser.
 *
 * @todo Move to Mime Mail project.
 */
function phpmailer_preview() {
  global $user;

  // Suppress devel output in preview.
  $GLOBALS['devel_shutdown'] = TRUE;

  $mailkey = 'phpmailer_preview';
  // @see user_register_submit()
  $language = user_preferred_language($user);
  $variables = user_mail_tokens($user, $language);
  $variables['!password'] = 'test';
  $body = _user_mail_text('register_no_approval_required_body', $language, $variables);

  // Convert non-html messages.
  // @see backdrop_mail_wrapper()
  $format = FILTER_FORMAT_DEFAULT;
  if (module_exists('mimemail')) {
    $format = config_get('mimemail.settings', 'mimemail_format');
  }
  $body = check_markup($body, $format, FALSE);
  // @see mimemail_prepare()
  $body = theme('mimemail_message', $body, $mailkey);
  foreach (module_implements('mail_post_process') as $module) {
    $function = $module . '_mail_post_process';
    $function($body, $mailkey);
  }

  print $body;
  backdrop_exit();
}

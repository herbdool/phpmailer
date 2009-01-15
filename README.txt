/* $Id$ */

-- SUMMARY --

This module integrates PHPMailer with Drupal, both as native drupal_mail()
wrapper, and as part of the Mime Mail module.


For a full description of the module, visit the project page:
  http://drupal.org/project/phpmailer

To submit bug reports and feature suggestions, or to track changes:
  http://drupal.org/project/issues/phpmailer


-- REQUIREMENTS --

* PHPMailer library.


-- INSTALLATION --

* Download the PHPMailer library 1.x from

    http://sourceforge.net/project/showfiles.php?group_id=26031&package_id=17494

  and copy the files

    class.phpmailer.php
    class.smtp.php

  into the sub-directory 'phpmailer' of this module.

* Install as usual, see http://drupal.org/node/70151 for further information.


-- CONFIGURATION --

* Customize module settings in Administer >> Site configuration >> Mail.


-- CUSTOMIZATION --

* To send HTML mails, you need to additionally install the Mime Mail module from
  http://drupal.org/project/mimemail.


-- CONTACT --

Current maintainers:
* Stefan M. Kudwien (smk-ka) - http://drupal.org/user/48898
* Daniel F. Kudwien (sun) - http://drupal.org/user/54136

Credits:
* PHPMailer module: Stefan Kudwien (smk) - http://drupal.org/user/48898
* PHPMailer class: Brent R. Matzelle - http://phpmailer.sourceforge.net
* SMTP Authentication Support: Luke Last - http://drupal.org/project/smtp

This project has been sponsored by:
* UNLEASHED MIND
  Specialized in consulting and planning of Drupal powered sites, UNLEASHED
  MIND offers installation, development, theming, customization, and hosting
  to get you started. Visit http://www.unleashedmind.com for more information.


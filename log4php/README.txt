Apache log4php Integration
==========================


Maintainers
-----------

Erik Webb (http://drupal.org/user/273404)


Features
--------

This module provides simple integration with the Apache log4php library.

WARNING
-------

If you plan to view your log messages in a browser interface please use the
LoggerRendererDrupalStandardObject renderer. Messages logged by other renderers are not passed through Drupal's
check_plain() function and thus represent a security vector when viewed in a browser.


Requirements
------------

This module requires the log4php library version >= v2.3.0. This can be installed in one of two ways -

The preferred installation method is via PEAR -

  pear channel-discover pear.apache.org/log4php
  pear install log4php/Apache_log4php

To install the library using Drush Make, use the following snippet -

  libraries[log4php][download][type] = "get"
  libraries[log4php][download][url] = "http://www.apache.org/dyn/closer.cgi/logging/log4php/2.3.0/apache-log4php-2.3.0-src.tar.gz"
  libraries[log4php][download][md5] = "9e6e81511135a9dbf7f7946dd17dd768"
  libraries[log4php][download][subtree] = "src/main/php"
  libraries[log4php][directory_name] = "log4php"

Alternatively the main code files can be installed via the Libraries module. Move the src/main/php directory to sites/all/libraries/log4php.


Installation
------------

After creating an XML configuration file, include it in your settings.php file -

  <?php
    $conf['log4php_config'] = 'sites/all/modules/log4php/log4php.xml';
  ?>

The default log4php.xml file is a basic configuration to test functionality. This configuration only writes log entries to a file located in the temp directory (will only work on Linux/Unix/Mac for now).


Recommended modules
-------------------

Libraries (1.x only, for now)


Advanced features
-------------------

* This module provides its own renderers which provide different ways of
 rendering an event log. See the included log4php.xml for an example or read
 more in log4php's documentation about Renderers.

* This module provides a way of overriding at runtime the default configuration
specified in settings.php. The example below changes the log level to DEBUG
for the root logger (and hence for all loggers that do not specifically set
their own log level):

  function mymodule_boot() {
    // NB: in this case it is the developer's responsibility to include
    // the required Logger.php file
    require_once('PATH_TO_LOG4PHP/src/main/php/Logger.php');

    // tell log4php_watchdog not to reinitialise the config
    $logger_config_inited = &drupal_static('log4php_config_inited');
    Logger::configure($GLOBALS['conf']['log4php_config']);
    $logger_config_inited = TRUE;
    // set the log level to DEBUG
    $root_logger = Logger::getRootLogger();
    $level = $root_logger->getLevel();
    if ($level->toInt() > LoggerLevel::DEBUG) {
      $debug_level = LoggerLevel::getLevelDebug();
      $root_logger->setLevel($debug_level);
    }
  }

